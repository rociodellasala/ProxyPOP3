#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>

typedef int file_descriptor;

#define MAXIMUM_MUA_CONNECTIONS 20

#define DATA_BUFFER 1024

void handle_connections(file_descriptor mua_tcp_socket) {
    int i = 0;
    int client_socket[MAXIMUM_MUA_CONNECTIONS];
    int activity;
    fd_set readfds;

    char buffer[DATA_BUFFER];

    const struct timespec timeout={}; /* ? */

    while(1) {
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
        }

        /* Sockets servidor */
        if(FD_ISSET(mua_tcp_socket, &readfds)) {
            /* Un nuevo cliente solicita conexion. Aceptarla aqui */
        }

        /* Clientes */
        if(FD_ISSET(client_socket[i], &readfds)) {
            if(read(client_socket[i], buffer, 1024) == 0) {
                /* Se ha leido un dato del cliente correctamente. Hacer aqui el tratamiento del mensaje */
            } else {
                /* Hay un error en la lectura. Quiza el cliente cerro la conexión. Tratar el error */
            }
        }
    }
}

