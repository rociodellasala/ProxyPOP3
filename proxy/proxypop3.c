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
#include "include/optionsParser.h"
#include "include/proxypop3.h"
#include "include/selector.h"

/* http://es.tldp.org/Tutoriales/PROG-SOCKETS/prog-sockets.html */

/* Creates a TCP/SCTP (specified in protocol) socket connection to the pop3 proxy server */
file_descriptor new_socket(int protocol, int port, struct sockaddr_in * address) {
    file_descriptor master_socket;

    /* Construct local address structure */
    memset(address, 0, sizeof(*address));           // Zero out structure
    (*address).sin_family = AF_INET;                // IPv4 address family
    (*address).sin_addr.s_addr = INADDR_ANY;        // Any incoming interface
    (*address).sin_port = htons((uint16_t) port);

    /* Creates a reliable stream master socket */
    master_socket = socket(AF_INET, SOCK_STREAM, protocol);

    if (master_socket < 0) {
        fprintf(stderr, "Unable to create %s socket", protocol == IPPROTO_TCP? "TCP" : "SCTP");
        perror("");
        exit(EXIT_FAILURE);
    }

    /* Binds the socket to the specified address (localhost port)  */
    if (bind(master_socket, (struct sockaddr*) address, sizeof(*address)) < 0) {
        perror("Unable to bind socket");
        exit(EXIT_FAILURE);
    }

    /* Returns the file descriptor of the socket */
    return master_socket;
}

void initialize_sockets(options opt) {
    struct sockaddr_in mua_address;
    struct sockaddr_in admin_address;

    file_descriptor mua_tcp_socket = new_socket(IPPROTO_TCP, opt.port, &mua_address);
    file_descriptor admin_sctp_socket = new_socket(IPPROTO_SCTP, opt.management_port, &admin_address);

    /* Mark the socket as a passive one so it will listen for incoming connections */
    if (listen(mua_tcp_socket, MAXIMUM_MUA_CONNECTIONS) < 0) {
        fprintf(stderr, "Unable to listen on port %d", opt.port);
        perror("");
        exit(EXIT_FAILURE);
    }

    printf("Listening on TCP port %d\n", opt.port);

    /* Mark the socket as a passive one so it will listen for incoming connections */
    if (listen(admin_sctp_socket, 1) < 0) {
        fprintf(stderr, "Unable to listen on port %d", opt.management_port);
        perror("");
        exit(EXIT_FAILURE);
    }

    printf("Listening on SCTP port %d\n", opt.management_port);
    printf("Waiting for connections ...\n");

    /* handle_connections(mua_tcp_socket, mua_address); */
}

/* Server ---> PROXY <--- Client/s */
int main(int argc, char ** argv) {
    options opt;
    int i;

    if (parse_input(argc,argv) < 0) {
        return -1;
    }

    opt = initialize_values(opt);
    opt = set_options_values(opt, argc, argv);

    initialize_sockets(opt);
}

