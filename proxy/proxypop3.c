#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "include/proxypop3.h"
#include "include/input_parser.h"
#include "include/handler.h"
#include "include/clients.h"
#include "include/main.h"
#include "include/administrator.h"

void initialize_proxy(options opt, file_descriptor mua_tcp_socket, struct sockaddr_in mua_address,
               file_descriptor admin_sctp_socket, struct sockaddr_in admin_address){
    fd_set read_fds;
    fd_set write_fds;

    const struct timespec timeout   = {
            .tv_sec         = 10,
            .tv_nsec        = 0
    };

    fd_handler fd_proxy = {
            .read_fds       = read_fds,
            .write_fds      = write_fds
    };

    proxy_pop3 proxy                = {
            .opt            = opt,
            .mua_socket     = mua_tcp_socket,
            .mua_addr       = mua_address,
            .admin_socket   = admin_sctp_socket,
            .admin_addr     = admin_address,
            .fds            = fd_proxy
    };

    handle_connections(proxy, timeout);
}

int create_server_socket(char * origin_server, int origin_port) {
    file_descriptor sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int optval = 1;

    if (sock < 0) {
        fprintf(stderr, "Unable to create server socket");
        perror("");
        exit(EXIT_FAILURE);
    }

    /* Construct the server address structure */
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((uint16_t) origin_port);

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0) {
        perror("'setsockopt()' failed");
        exit(EXIT_FAILURE);
    }

    /* Establish the connection to the server */
    if (connect(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection to origin server failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Success! Connected to server at %s\n", origin_server);
    }

    return sock;
}


clients_list accept_user_connection(proxy_pop3 * proxy, clients_list clients){
    int new_socket;
    int server_fd;
    client c;
    int addresslen                  = sizeof(proxy->mua_addr);
    char * g_message                = "Welcome to Proxy POP3 1.0\r\n";

    if ((new_socket = accept((*proxy).mua_socket, (struct sockaddr *)&(*proxy).mua_addr, (socklen_t *)&addresslen))<0) {
        perror("Accept error ocurred!\n");
        exit(EXIT_FAILURE);
    }

    printf("\n[USER] New connection:\n\t - Socket FD: %d \n\t - IP: %s \n\t - Port: %d \n", new_socket, inet_ntoa((*proxy).mua_addr.sin_addr), ntohs((*proxy).mua_addr.sin_port));

    /* Send new connection greeting message */
    if (send(new_socket, g_message, strlen(g_message), 0) != strlen(g_message)) {
        perror("An error ocurred trying to send greeting messsage!");
    } else {
        printf("Greeting message was sent successfully to socket %d\n", new_socket);
    }

    /* Add new socket to list of sockets */
    if ((server_fd = create_server_socket((*proxy).opt.origin_server, (*proxy).opt.origin_port)) == 0) {
        exit(EXIT_FAILURE);
    }

    c.client_fd = new_socket;
    c.server_fd = server_fd;

    add_client(clients, c);

    return clients;
}

void remove_user_connection(struct sockaddr_in address, client cl, clients_list clients) {
    int addrlen = sizeof(address);

    /* Somebody disconnected, get his details and print */
    getpeername(cl.client_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    printf("\nClient disconnected: \n\t - Socket FD: %d \n\t - IP: %s \n\t - Port: %d \n", cl.client_fd, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    remove_client(clients, cl);

    /* Close the socket and mark as 0 in list for reuse */
    close(cl.client_fd);
    close(cl.server_fd);
    cl.server_fd = 0;
    cl.client_fd = 0;
}


file_descriptor add_client_to_set(fd_set * readfds, clients_list clients, file_descriptor max_fd) {
    file_descriptor client_fd, server_fd;
    node_c * temp = clients->first;
    while(temp != NULL) {
        client_fd = temp->c.client_fd;
        server_fd = temp->c.server_fd;

        /* If valid socket descriptor then add to read list */
        if (client_fd > 0) {
            FD_SET(client_fd, readfds);
        }

        if (server_fd > 0) {
            FD_SET(server_fd, readfds);
        }

        if (client_fd > max_fd) {
            max_fd = client_fd;
        }

        if (server_fd > max_fd) {
            max_fd = server_fd;
        }

        temp = temp->next;
    }

    return max_fd;
}

file_descriptor add_admin_to_set(fd_set * readfds, admin admin, file_descriptor max_fd) {
    file_descriptor admin_fd = admin.admin_fd;
 
    /* If valid socket descriptor then add to read list */
    if (admin_fd > 0) {
        FD_SET(admin_fd, readfds); 
    }

    if (admin_fd > max_fd) {
        max_fd = admin_fd; 
    }

    return max_fd;
}