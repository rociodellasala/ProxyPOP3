#ifndef PROXYPOP3_MAIN_H
#define PROXYPOP3_MAIN_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BACKLOG 10

/* Typedefs */
typedef struct {
    int port;                           // -p
    char * error_file;                  // -e
    char * listen_address;              // -l
    char * management_address;          // -L
    int management_port;                // -o
    char * replacement_msg;             // -m
    char * filtered_media_types;        // -M
    char * origin_server;               // This is the argument origin_server, it's not an option
    int origin_port;                    // -P
    char *filter_command;               // -t
} options;

typedef struct sockaddr_in address;

typedef int file_descriptor;

typedef struct {
    fd_set read_fds;
    fd_set write_fds;
} fd_handler;

typedef struct {
    options opt;
    file_descriptor mua_socket;
    address mua_addr;
    file_descriptor admin_socket;
    address admin_addr;
    fd_handler fds;
} proxy_pop3;


#include "input_parser.h"

/* Functions */
void initialize_sockets(options);
file_descriptor new_socket(int, int, struct sockaddr_in *);

#endif //PROXYPOP3_MAIN_H
