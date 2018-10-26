#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include "include/input_parser.h"

int validate_address(const char * parameter) {
    if(parameter){

    }
    /* TODO */
    return 0;
}


int validate_port(const char * parameter) {
    if(parameter){

    }
    /* TODO */
    return 0;
}

int validate_parameters(const char * next_option, const char * parameter) {
    if (strcmp(next_option, "-L") == 0) {
        if (validate_address(parameter) < 0) {
            printf("Invalid management address\n");
            return -1;
        }
    } else if (strcmp(next_option, "-o") < 0) {
        if (validate_port(parameter) != 0) {
            printf("Invalid management port\n");
            return -1;
        }
    }
}

int validate_options(int argc, char ** argv) {
    int arg;
    int option;
    char * next_option = NULL;
    char * parameter;

    opterr = 0;

    for (arg = 1; arg < argc; arg++) {
        if (optind < argc) {
            next_option = argv[optind];
            if (next_option[0] != '-') {
                printf("'%s' is wrong, options must begin with '-', for example: -m [parameter]\n", next_option);
                return -1;
            }
        }

        option = getopt(argc, argv, "L:o:");

        parameter = optarg;

        if (parameter != NULL) {
            if (parameter[0] == '-') {
                printf("Parameters can't begin with '-'\n");
                return -1;
            }
        }

        if (option == ':') {
            printf("Missing option argument for '%s'\n", next_option);
            return -1;
        } else if (option == '?') {
            printf("Invalid option: '%s'\n", next_option);
            return -1;
        } else if (option != -1) {
            printf("Option argument for '%s' is %s\n", next_option, parameter);
            if(validate_parameters(next_option, parameter) < 0){
                return -1;
            }
        }
    }

    printf("No errors found on input, starting to run client\n");
}

void print_usage() {
    printf("USAGE: ./configurationClient [ POSIX style options ] \n"
           "POSIX style options: \n"
           "\t-L [management address]: Specifies the address where the management service will serve. By default is localhost. \n"
           "\t-o [management port]: Specifies SCTP port where the management server is located. By default is 9090. \n");
}

int parse_input(const int argc, char **argv) {
    if (validate_options(argc, argv) < 0) {
        print_usage();
        return -1;
    }

    return 0;
}

void initialize_values() {
    parameters                         = malloc(sizeof(*parameters));
    parameters->management_address     = "127.0.0.1";
    parameters->management_port        = 9090;
}

options set_options_values(const int argc, char ** argv, options opt) {
    int c;
    optind = 1;

    while ((c = getopt(argc, argv, "L:o:")) != -1) {
        switch (c) {
            case 'L':
                parameters->management_address = optarg;
                break;
            case 'o':
                parameters->management_port = atoi(optarg);
                break;
            default: /* Si validate_option funciona correctamente nunca entrará aqui */
                break;
        }
    }

    return opt;
}

