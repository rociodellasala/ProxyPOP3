#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include "include/selector.h"

void handle_connections(file_descriptor mua_tcp_socket, struct sockaddr_in address) {
    int i = 0;
    int client_socket[MAXIMUM_MUA_CONNECTIONS];
    int activity;
    int new_socket;
    int valread;
    fd_set readfds;

    char buffer[DATA_BUFFER];

    const struct timespec timeout={}; /* ? */
    int addrlen = sizeof(address);


    while(1) {
        printf("ACA");
        /* Initializes the file descriptor set fdset to have zero bits for all file descriptors */
        FD_ZERO(&readfds);

        FD_SET(mua_tcp_socket, &readfds);
        for(i = 0; i < MAXIMUM_MUA_CONNECTIONS; i++) {
            FD_SET(client_socket[i], &readfds);
        }

        /* Waits until something happens */
        activity = select(MAXIMUM_MUA_CONNECTIONS+1, &readfds, NULL, NULL, NULL);

        if(activity < 0) {
            // Caso de error
            printf("Select error");
        }

        /* Sockets servidor */
        if(FD_ISSET(mua_tcp_socket, &readfds)) {
            if(new_socket = accept(mua_tcp_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen) < 0){
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

            if(send(new_socket, "hola", strlen("hola"), 0) != strlen("hola")){
                perror("send");
            }

            puts("Welcome message sent successfully");

            for(i = 0; i < 20; i++){
                if(client_socket[i] == 0){
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }

        }

        for(i = 0; i < 20; i++) {
            int sd = client_socket[i];
            /* Clientes */
            if (FD_ISSET(sd, &readfds)) {
                if ((valread= read(sd, buffer, 1024)) == 0) {
                    /* Se ha leido un dato del cliente correctamente. Hacer aqui el tratamiento del mensaje */
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                } else {
                    /* Hay un error en la lectura. Quiza el cliente cerro la conexión. Tratar el error */
                    //set the string terminating NULL byte on the end of the data read
                    buffer[valread] = '\0';
                    send(sd , buffer , strlen(buffer) , 0 );
                }
            }
        }
    }
}

