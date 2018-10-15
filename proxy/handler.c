#include <errno.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include "include/main.h"
#include "include/handler.h"
#include "include/proxypop3.h"
#include "include/clients.h"
#include "include/administrator.h"


void handle_connections(proxy_pop3 proxy, struct timespec timeout) {
    int max_sd;
    int activity;
    int clientfd, serverfd;
    clients_list clients = new_list();
    admin administrator;
    char buffer[BUFFER_SIZE];
    int running                     = 1;
    node_c * temp;

    while(running) {
        /* Clear the read set of file descriptors (initialize it) */
        FD_ZERO(&proxy.fds.read_fds);

        /* Add mua socket to read set*/
        FD_SET(proxy.mua_socket, &proxy.fds.read_fds);
        /* Add admin socket to read set*/
        FD_SET(proxy.admin_socket, &proxy.fds.read_fds);

        /* Add client sockets to read set */
        add_to_set(&proxy.fds.read_fds, clients);

        if(is_empty(clients) == 0){
            max_sd = proxy.mua_socket;
        } else {
            max_sd = clients->last->c.client_fd;
        }

        /* Wait for an activity on one of the sockets. If timeout is NULL, waits indefinitely */
        activity = select(max_sd + 1, &proxy.fds.read_fds, NULL, NULL, /*&timeout*/ NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("Select error occurred!\n");
        }

        /* If something happened on the MUA socket, its an incoming connection of a client */
        if (FD_ISSET(proxy.mua_socket, &proxy.fds.read_fds)) {
            clients = accept_user_connection(&proxy, clients);
        }

        /* If something happened on the ADMIN socket, its an incoming connection of a client */
        if (FD_ISSET(proxy.admin_socket, &proxy.fds.read_fds)) {
            administrator = accept_admin_connection(&proxy);
        }

        temp = clients->first;
        print_current_clients(clients);

        /* Else its other socket (already connected clients or server) */
        while(temp != NULL) {
            clientfd = temp->c.client_fd;
            serverfd = temp->c.server_fd;
            ssize_t valread;
            /* If a client descriptor was set */
            if (FD_ISSET(clientfd, &proxy.fds.read_fds)) {
                /* Check if it was for closing also read the incoming message */
                if ((valread = read(clientfd, buffer, BUFFER_SIZE)) == 0) {
                    /* An error ocurred while reading. Possibly the client has closed the connection. */
                    remove_connection(proxy.mua_addr, temp->c, clients);
                } else {
                    /* A client data has been read correctly. Set the string terminating NULL byte on the end of the data read */
                    buffer[valread] = '\0';
                    printf("Mensaje del FD %d es: %s\n", clientfd, buffer);
                    //parse_command();
                }
                /* If a server descriptor was set */
            } else if (FD_ISSET(serverfd, &proxy.fds.read_fds)) {
                if ((valread = read(serverfd, buffer, BUFFER_SIZE)) == 0) {
                    remove_connection(proxy.mua_addr, temp->c, clients);
                } else {
                    /* Set the string terminating NULL byte on the end of the data read */
                    buffer[valread] = '\0';
                    send(clientfd, buffer, strlen(buffer), 0);
                }
            }
            temp = temp->next;
        }

    }
}
