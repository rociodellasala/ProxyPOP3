#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "include/selector.h"
#include "include/optionsParser.h"

int create_server_socket(char * origin_server, int origin_port) {
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock < 0) {
        fprintf(stderr, "Unable to create server socket");
        perror("");
        exit(EXIT_FAILURE);
    }

    /* Construct the server address structure */
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(origin_port);

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection to origin server failed");
        exit(EXIT_FAILURE);
    }

    return sock;
}

int add_to_set(fd_set *readfds, int max_sd, const int sockets[], int max_clients) {
    int i;
    int sd;

    for (i = 0 ; i < max_clients ; i++) {
        sd = sockets[i];

        /* If valid socket descriptor then add to read list */
        if (sd > 0) {
            FD_SET(sd, readfds);
        }

        // Highest file descriptor number, need it for the select function */
        if (sd > max_sd) {
            max_sd = sd;
        }
    }

    return max_sd;
}


void end_connection(struct sockaddr_in address, int client, int sd, int *client_socket, int *server_socket) {
    int addrlen = sizeof(address);

    /* Somebody disconnected , get his details and print */
    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    printf("Client disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    /* Close the socket and mark as 0 in list for reuse */
    close(sd);
    close(server_socket[client]);
    client_socket[client] = 0;
    server_socket[client] = 0;
}

void handle_connections(options opt, file_descriptor mua_tcp_socket, struct sockaddr_in address) {
    int i = 0;
    int max_sd;
    int activity;
    int new_socket;
    int valread;

    int client_socket[FD_SETSIZE] = {0};
    int server_socket[FD_SETSIZE] = {0};

    fd_set readfds;
    int max_clients = FD_SETSIZE;
    char buffer[BUFFER_SIZE];
    printf("port: %d\n",address.sin_port);
    const struct timespec timeout={}; /* ? */
    int addrlen = sizeof(address);
    printf("addrlen: %d\n",addrlen);

    char * message = "Welcome to Proxy POP3 1.0\r\n";

    while(1) {

        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(mua_tcp_socket, &readfds);
        int max_sd = mua_tcp_socket;
        max_sd = add_to_set(&readfds, max_sd, client_socket, max_clients);
        //max_sd = add_to_set(&readfds, max_sd, server_socket, max_clients);
        printf("port: %d\n",address.sin_port);
        // Wait for an activity on one of the sockets. Timeout is NULL, so
        // wait indefinitely
        printf("max: %d", max_sd);
        int activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
        printf("activity: %d\n", activity);
        if ((activity < 0) && (errno!=EINTR)) {
            printf("select error");
        }

        // If something happened on the master socket, then its an incoming
        // connection
        if (FD_ISSET(mua_tcp_socket, &readfds)) {
            int new_socket;
            printf("master: %d\n", mua_tcp_socket);
            if (address.sin_addr.s_addr == INADDR_ANY){
                printf("bien\n");
            }

            if(address.sin_family == AF_INET){
                printf("bien\n");
            }
            if ((new_socket = accept(mua_tcp_socket, (struct sockaddr *)&address,
                                     (socklen_t*)&addrlen))<0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            printf("new socket: %d\n", new_socket);

            //inform user of socket number - used in send and receive commands
            printf("New connection, socket fd is %d , ip is : %s , port : %d \n",
                   new_socket,
                   inet_ntoa(address.sin_addr),
                   ntohs(address.sin_port));

            //send new connection greeting message
            if( send(new_socket, message, strlen(message), 0) !=
                strlen(message) ) {
                perror("send");
            }

            puts("Welcome message sent successfully");

            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++) {
                //if position is empty
                if ( client_socket[i] == 0 ) {
                    client_socket[i] = new_socket;
                    printf("Adding to list of client sockets as %d\n" , i);
                    //Create pop3 socket
                    if ((server_socket[i] = create_server_socket(
                            opt.origin_server, opt.origin_port)) == 0){
                        perror("socket failed");
                        exit(EXIT_FAILURE);
                    }
                    break;
                }
            }
        }

        //else its some IO operation on some other socket :)
        for (i = 0; i < max_clients; i++) {
            int server_sd = server_socket[i];
            int client_sd = client_socket[i];
            ssize_t valread;

            // if a client descriptor was set
            if (FD_ISSET(client_sd, &readfds)) {
                //Check if it was for closing and also read the incoming message
                if ((valread = read(client_sd, buffer, BUFFER_SIZE)) == 0) {
                    end_connection(address, i, client_sd, client_socket,
                                   server_socket);
                }
                    //Send to server
                else {
                    // set the string terminating NULL byte on the end of the
                    // data read
                    buffer[valread] = '\0';
                    send(server_sd, buffer, strlen(buffer), 0);
                }
                // if a server descriptor was set
            } else if (FD_ISSET(server_sd, &readfds)) {

                if ((valread = read(server_sd, buffer, BUFFER_SIZE)) == 0) {
                    end_connection(address, i, server_sd, client_socket,
                                   server_socket);
                }
                    //Send to client
                else {
                    // set the string terminating NULL byte on the
                    // end of the data read
                    buffer[valread] = '\0';
                    send(client_sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }
}

