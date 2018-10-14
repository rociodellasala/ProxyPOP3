#include "include/optionsParser.h"

int parse_input(int argc, char ** argv) {
    if (validate_options(argc, argv) < 0) {
        print_usage();
        return -1;
    }

    return 0;
}

int validate_options(int argc, char ** argv) {
    int arg;
    int option;
    char * next_option;
    char * parameter;

    opterr = 0;

    for (arg = 1; arg < argc; arg++) {
        if (optind < argc) {
            next_option = argv[optind];
            if (next_option[0] != '-') {
                printf("'%s' is wrong, options must begin with '-'. For example: -m [parameter].\n", next_option);
                return -1;
            }
        }

        option = getopt(argc, argv, "L:o:");

        parameter = optarg;

        if (parameter != NULL) {
            if (parameter[0] == '-') {
                printf("Parameters can't begin with '-'.\n");
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

    /* For debug, remember to take it out ! */
    printf("No errors found on input ! \n");
}

int validate_parameters(char * next_option, char * parameter) {
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

int validate_address(char * parameter) {
    /* TODO */
    return 0;
}


int validate_port(char * parameter) {
    /* TODO */
    return 0;
}

void print_usage() {
    printf("USAGE: ./pop3client [ POSIX style options ] \n");
    printf("POSIX style options: \n");
    printf("\t-L [management address]: Specifies the address where the management service will serve. \n");
    printf("\t-o [management port]: Specifies SCTP port where the management server is located. By default is 9090. \n");
}

options initialize_values(options opt) {
    opt.management_address     = "127.0.0.1";
    opt.management_port        = 9090;
    return opt;
}

options set_options_values(options opt, int argc, char ** argv) {
    int c;
    optind = 1;

    while ((c = getopt(argc, argv, "L:o:")) != -1) {
        switch (c) {
            case 'L':
                opt.management_address = optarg;
                break;
            case 'o':
                opt.management_port = atoi(optarg);
                break;
            default: /* If validate_option works correctly, it won't enter here */
                break;
        }
    }

    return opt;
}

