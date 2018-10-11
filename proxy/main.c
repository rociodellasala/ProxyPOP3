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

/* Server ---> PROXY <--- Client/s */

int main(int argc, char ** argv){

    options opt;
    int i;

    if(parse_input(argc,argv) < 0){
        return -1;
    }

    opt = initialize_values(opt);

    opt = set_options_values(opt, argc, argv);

    /* For debug, remember to take it out ! (Ale cuando corro el tp lo corro con los argumentos -m holaaaa origin_server para
     * chequear aca que se este cambiando en options ese valor */
    printf("%s", opt.replacement_msg);


}

