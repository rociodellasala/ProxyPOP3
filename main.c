#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>

typedef struct {
    int port;
    char * error_file;
    char * listen_address;
    char * management_address;
    int management_port;
    char * replacement_msg;
    char * filtered_media_types;
    char * origin_server;
    int origin_port;
    char *filter_command;

} options;

#define TRUE 1
#define BUFFER_SIZE 1024

int new_master_socket(int protocol, struct sockaddr_in * addr) {
    int master_socket;
    struct sockaddr_in address = * addr;

    /* Creates a reliable stream master socket */
    master_socket = socket(AF_INET, SOCK_STREAM, protocol);

    if (master_socket < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    /* Binds the socket to localhost port  */
    if (bind(master_socket, (struct sockaddr*) &address, sizeof(address)) < 0) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    /* Returns the file descriptor of the socket */
    return master_socket;
}


int main(int argc, char ** argv){
    int port = 1110; /* SACAR */
    //options opt = parse_options(argc,argv);

    int max_clients = FD_SETSIZE;
    /* Initialise all client_socket[] and server_socket[] to 0 so not checked */
    int client_socket[FD_SETSIZE] = {0};
    int server_socket[FD_SETSIZE] = {0};
    struct sockaddr_in address;

    char buffer[BUFFER_SIZE];

    /* Set of sockets to "monitor" for some activity */
    /* https://www.binarytides.com/multiple-socket-connections-fdset-select-linux/ */
    fd_set readfds;

    char * message = "Hello World\n";

    struct sockaddr_in address_proxy;

    address_proxy.sin_family        = AF_INET;
    address_proxy.sin_addr.s_addr   = INADDR_ANY;
    address_proxy.sin_port          = htons(port);

    int master_tcp_socket = new_master_socket(IPPROTO_TCP, &address_proxy);

    if(listen(master_tcp_socket, 3) < 0){
        perror("listen failure ");
        exit(EXIT_FAILURE);
    }

    printf("Listening on TCP port %d\n", port);

    int addrlen = sizeof(address);
    printf("Waiting for connections ...\n");

    /* Handle multiple connections at a time (multiple clients) */
    while(TRUE){


    }
}

