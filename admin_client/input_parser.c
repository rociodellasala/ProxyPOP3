#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include "include/input_parser.h"

void print_usage() {
    printf("USAGE: ./admin_client <server-port> <server-address> \n");
}

int parse_input(const int argc, char **argv) {
    if (argc != 3) {
        perror("Program execution requires two parameters: address and port of server\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    set_options_values(argc, argv);

    return 0;
}

void set_options_values(const int argc, char ** argv) {
    clt_parameters                      = malloc(sizeof(*clt_parameters));
    clt_parameters->server_address      = argv[argc-1];
    clt_parameters->server_port         = (uint16_t) atoi(argv[argc - 2]);
}

