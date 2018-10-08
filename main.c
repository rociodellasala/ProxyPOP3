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
    int origin_port;
    char * origin_server;
    char * listen_address;
} options;

#define TRUE 1
#define BUFFER_SIZE 1024
#define PENDING_CONNECTIONS 3


/* Creates a TCP socket connection to the pop3 server */
int new_master_socket(int protocol, struct sockaddr_in * address) {
    int master_socket;
    struct sockaddr_in addr = * address;

    /* Creates a reliable stream master socket */
    master_socket = socket(AF_INET, SOCK_STREAM, protocol);

    if (master_socket < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    /* Binds the socket to localhost port  */
    if (bind(master_socket, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    /* Returns the file descriptor of the socket */
    return master_socket;
}

options parse_input(int argc, char ** argv){
    int c;
    int index = 0;

    /* Initialize default values */
    options opt = {
            .port                   = 1110,
            .listen_address         = INADDR_ANY,
            .origin_port            = 110
    };

    if (argc == 2){
        opt.origin_server = argv[index];
    } else {
        fprintf(stderr, "Usage: %s <origin-server>\n", argv[0]);
        exit(1);
    }

    return opt;
}


/* Server ---> PROXY <--- Client/s */

int main(int argc, char ** argv){

    int max_clients = 30;
    /* Initialise all client_socket[] and server_socket[] to 0 so not checked */
    int client_socket[FD_SETSIZE] = {0};
    int server_socket[FD_SETSIZE] = {0};

    char buffer[BUFFER_SIZE];

    /* Set of sockets to "monitor" for some activity */
    /* https://www.binarytides.com/multiple-socket-connections-fdset-select-linux/ */
    /* https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/ */
    fd_set readfds;

    options opt = parse_input(argc,argv);

    /* Construct the address structure */
    struct sockaddr_in address_proxy;

    address_proxy.sin_family        = AF_INET;
    address_proxy.sin_addr.s_addr   = INADDR_ANY;
    address_proxy.sin_port          = htons(opt.port);

    int master_tcp_socket = new_master_socket(IPPROTO_TCP, &address_proxy);

    if(listen(master_tcp_socket, PENDING_CONNECTIONS) < 0){
        perror("listen failure ");
        exit(EXIT_FAILURE);
    }

    printf("Listening on TCP port %d\n", opt.port);

    /* Accept the incoming connection */
    int addrlen = sizeof(address_proxy);
    printf("Waiting for connections ...\n");

    /* Handle multiple connections at a time (multiple clients) */
    while(TRUE)
    {
        //Completar
    }
}

