#include "include/optionsParser.h"

int parse_input(int argc, char ** argv) {
    if (argc < 2) {
        printf("Program execution requires at least one parameter \n");
        print_usage();
        return -1;
    }

    if (strcmp(argv[1], "-h") == 0) {
        if(argc >= 3){
            print_usage();
            return -1;
        }
        print_help();
        exit(0);
    }

    if (strcmp(argv[1], "-v") == 0) {
        if (argc >= 3) {
            print_usage();
            return -1;
        }
        print_version();
        exit(0);
    }

    if (validate_options(argc, argv) < 0 || validate_origin_server_argument(argv[argc-1]) < 0) {
        print_usage();
        return -1;
    }

    return 0;
}

void print_usage() {
    printf("USAGE: ./pop3filter [ POSIX style options ] <origin-server> \n");
    printf("POSIX style options: \n");
    printf("\t-e [error file]: Specifies the file where to redirect stderr. By default is '\\dev\\null'. \n");
    printf("\t-h : Prints out help and ends. \n");
    printf("\t-l [listen address]: Specifies the address where the proxy will serve. \n");
    printf("\t-L [management address]: Specifies the address where the management service will serve. \n");
    printf("\t-m [message of replacement]: Specifies the message to replace filtered text(option -M). \n");
    printf("\t-M [filtered media-type]: Specifies a media types to be censored. \n");
    printf("\t-o [management port]: Specifies SCTP port where the management server is located. By default is 9090. \n");
    printf("\t-p [port]: Specifies TCP port where to listen for incoming POP3 connections. By default is 1110. \n");
    printf("\t-P [origin port]: Specifies TCP port where the POP3 server is located on the server. By default is 110. \n");
    printf("\t-t [filtered command]: Command used for external transformations. By default applies no transofrmations. \n");
    printf("\t-v : Prints out the proxy version and ends. \n");
    printf("<origin-server>: Address of POP3 origin server.\n");
}

void print_help() {
    printf(" --- HELP ---\n");
    print_usage();
}

void print_version() {
    printf("Version: POP3 Proxy 1.0\n");
}

int validate_options(int argc, char ** argv) {
    int arg;
    int option;
    char * next_option;
    char * parameter;
    int size            = argc-1;
    char ** options     = (char **) malloc(size * sizeof(char *));;

    for (arg = 0; arg < size; arg++) {
        options[arg] = malloc(sizeof(char) * strlen(argv[arg]) + 1);
        strcpy(options[arg], argv[arg]);
    }

    opterr = 0;

    for (arg = 1; arg < size; arg++) {
        if (optind < size) {
            next_option = options[optind];

            if (next_option[0] != '-') {
                printf("'%s' is wrong, options must begin with '-'. For example: -m [parameter].\n", next_option);
                free_options(options, size);
                return -1;
            }
        }

        option = getopt(size, options, ":e:l:L:m:M:o:P:p:t:");

        parameter = optarg;

        if (parameter != NULL) {
            if (parameter[0] == '-') {
                printf("Parameters can't begin with '-'.\n");
                free_options(options, size);
                return -1;
            }
        }

        if (option == ':') {
            printf("Missing option argument for '%s'\n", next_option);
            free_options(options, size);
            return -1;
        } else if (option == '?') {
            printf("Invalid option: '%s'\n", next_option);
            free_options(options, size);
            return -1;
        } else if (option != -1) {
            printf("Option argument for '%s' is %s\n", next_option, parameter);
            if(validate_parameters(next_option, parameter) < 0){
                return -1;
            }
        }
    }

    free_options(options, size);

    /* For debug, remember to take it out ! */
    printf("No errors found on input ! \n");
}

void free_options(char ** options, int size) {
    int i;

    for (i = 0; i < size ; i++) {
        free(options[i]);
    }

    free(options);
}

int validate_parameters(char * next_option, char * parameter) {
    if (strcmp(next_option, "-l") == 0) {
        if (validate_address(parameter) != 0) {
            printf("Invalid listen address\n");
            return -1;
        }
    } else if (strcmp(next_option, "-L") == 0) {
        if (validate_address(parameter) != 0) {
            printf("Invalid management address\n");
            return -1;
        }
    } else if (strcmp(next_option, "-e") == 0) {
        if (validate_error_file(parameter) != 0){
            printf("Invalid error file\n");
            return -1;
        }
    } else if (strcmp(next_option, "-m") == 0) {
        if (validate_message(parameter) != 0) {
            printf("Invalid message of replacement\n");
            return -1;
        }
    } else if (strcmp(next_option, "-M") == 0) {
        if (validate_media_type(parameter) != 0){
            printf("Invalid media type\n");
            return -1;
        }
    } else if (strcmp(next_option, "-o") == 0) {
        if (validate_port(parameter) != 0) {
            printf("Invalid management port\n");
            return -1;
        }
    } else if (strcmp(next_option, "-p") == 0) {
        if (validate_port(parameter) != 0) {
            printf("Invalid port\n");
            return -1;
        }
    } else if (strcmp(next_option, "-P") == 0) {
        if (validate_port(parameter) != 0) {
            printf("Invalid origin port\n");
            return -1;
        }
    } else if (strcmp(next_option, "-t") == 0) {
        if (validate_transformation(parameter) != 0) {
            printf("Invalid transformation\n");
            return -1;
        }
    }
}

/* Ale: Si cambias algo en esta funcion o en validate_port --> en clients esta copypasteada la misma, cambiar ahi tmb */
int validate_address(char * parameter) {
    if (strcmp(parameter, "localhost") == 0) {
        return 0;
    } else if (is_valid_ip(parameter) == 0) {
        return 0;
    }

    return -1;
}

int validate_error_file(char * parameter) {
    /* no se si se podra hacer asi
    FILE *fb = fopen("parameter","r");
    if(fb==NULL)
        return -1;
    else
        return 0;
    */
    return 0; //TODO
}

int validate_message(char * parameter) {
    return 0; //TODO
}

int validate_media_type(char * parameter) {
    return 0; //TODO
}

/* Ale: Si cambias algo en esta funcion o en validate_address --> en clients esta copypasteada la misma, cambiar ahi tmb */
int validate_port(char * parameter) {
    int i;
    if (strlen(parameter) == 4) {
        for (i = 0 ; i < 4 ; i++ ) {
            if (!isdigit(parameter[i])) {
                return -1;
            }
        }
        return 0;
    }
    return -1;
}

int validate_transformation(char * parameter) {
    return 1; //TODO
}

/* Returns 0 if IP string is valid, else returns -1 */
int is_valid_ip(char * ip_str) {
    int num;
    char * ptr;
    int dots = 0;


    if (ip_str == NULL) {
        return -1;
    }

    ptr = strtok(ip_str, DELIM);

    if (ptr == NULL) {
        return -1;
    }

    while (ptr) {
        /* After parsing string, it must contain only digits */
        if (!valid_digit(ptr)) {
            return -1;
        }

        num = atoi(ptr);

        /* Check for valid IP */
        if (num >= 0 && num <= 255) {
            /* Parse remaining string */
            ptr = strtok(NULL, DELIM);
            if (ptr != NULL) {
                ++dots;
            }
        } else {
            return -1;
        }
    }

    /* Valid IP string must contain 3 dots */
    if (dots != 3) {
        return -1;
    }

    return 0;
}

/* Returns 0 if string contain only digits, else returns -1 */
int valid_digit(char * ip_str) {
    while (*ip_str) {
        if ((*ip_str) >= '0' && (*ip_str) <= '9') {
            ++ip_str;
        } else {
            return -1;
        }
    }

    return 0;
}

// TODO: Validate origin server argument
int validate_origin_server_argument(char * origin_server) {
    if (strcmp("localhost", origin_server) == 0) {
        return 0;
    } else if (is_valid_ip(origin_server) == 0) {
        printf("Valid IP!\n");
        return 0;
    }

    //TODO: VALIDATE IPV6 ??
    return -1;
}

options initialize_values(options opt) {
    opt.port                   = 1110;
    opt.error_file             = "/dev/null";
    opt.management_address     = "127.0.0.1";
    opt.management_port        = 9090;
    opt.listen_address         = INADDR_ANY;
    opt.replacement_msg        = "Parte reemplazada..";
    opt.filtered_media_types   = "text/plain,image/*"; /* Default value ?? */
    opt.origin_port            = 110;

    return opt;
}

options set_options_values(options opt, int argc, char ** argv) {
    int c;

    opt.origin_server   = argv[argc-1];
    optind              = 1;

    while ((c = getopt(argc, argv, "e:l:L:m:M:o:p:P:t:")) != -1) {
        switch (c) {
            case 'e':
                opt.error_file = optarg;
                break;
            case 'l':
                opt.listen_address = optarg;
                break;
            case 'L':
                opt.management_address = optarg;
                break;
            case 'm':
                opt.replacement_msg = optarg;
                break;
            case 'M':
                opt.filtered_media_types = optarg;
                break;
            case 'o':
                opt.management_port = atoi(optarg);
                break;
            case 'p':
                opt.port = atoi(optarg);
                break;
            case 'P':
                opt.origin_port = atoi(optarg);
                break;
            case 't':
                opt.filter_command = optarg;
                break;
            default: /* If validate_option works correctly, it won't enter here */
                break;
        }
    }

    return opt;
}





