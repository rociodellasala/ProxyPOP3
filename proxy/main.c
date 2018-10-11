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
#include "include/main.h"

/* Creates a TCP/SCTP socket connection to the pop3 server */
int new_socket(int protocol, int port) {
    int master_socket;
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons((uint16_t) port);

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


void initialize_sockets(options opt){
    struct sockaddr_in address_proxy;
    struct sockaddr_in address_management;


    file_descriptor mua_tcp_socket = new_socket(IPPROTO_TCP, opt.port);
    file_descriptor admin_sctp_socket = new_socket(IPPROTO_SCTP, opt.management_port);
}


/* Server ---> PROXY <--- Client/s */

int main(int argc, char ** argv) {

    options opt;
    int i;

    if(parse_input(argc,argv) < 0) {
        return -1;
    }

    opt = initialize_values(opt);
    opt = set_options_values(opt, argc, argv);

    /* For debug, remember to take it out ! (Ale cuando corro el tp lo corro con los argumentos -m holaaaa origin_server para
     * chequear aca que se este cambiando en options ese valor */
    printf("%s", opt.replacement_msg);

    /*initialize_sockets(opt);*/

}

