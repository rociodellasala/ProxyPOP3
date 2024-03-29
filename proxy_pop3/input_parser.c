#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include "include/input_parser.h"
#include "include/utils.h"
#include "include/filter_list.h"

void print_usage() {
    printf("USAGE: ./pop3filter [ POSIX style options ] <origin-server> \n"
           "POSIX style options: \n"
           "\t-e [error file]: Specifies the file where to redirect stderr. By default is '\\dev\\null'. \n"
           "\t-h : Prints out help and ends. \n"
           "\t-l [listen address]: Specifies the address where the proxy_pop3 will serve. \n"
           "\t-L [management address]: Specifies the address where the management service will serve. \n"
           "\t-m [message of replacement]: Specifies the message to replace filtered text(option -M). \n"
           "\t-M [filtered media-type]: Specifies a media types to be censored. \n"
           "\t-o [management port]: Specifies SCTP port where the management server is located. By default is 9090. \n"
           "\t-p [port]: Specifies TCP port where to listen for incoming POP3 connections. By default is 1110. \n"
           "\t-P [origin port]: Specifies TCP port where the POP3 server is located. By default is 110. \n"
           "\t-t [filtered command]: Command used for external transformations. By default applies no transformations. \n"
           "\t-v : Prints out the proxy_pop3 version and ends. \n"
           "<origin-server>: Address of POP3 origin server.\n");
}

void print_help() {
    printf("\n-------------------------- HELP --------------------------\n");
    print_usage();
}

void print_version() {
    printf("Version: POP3 Proxy 1.0\n");
}

void free_options(char ** options, int size) {
    int i;

    for (i = 0; i < size ; i++) {
        free(options[i]);
    }

    free(options);
}

int validate_error_file(const char * parameter) {
    if(parameter){

    }
    FILE * fb = fopen(parameter,"r");
    if(fb==NULL)
        return -1;
    else
        return 0;
}

int validate_media_type(char * parameter) {

    char *pt;
    pt = strtok (parameter,",");
    while (pt != NULL) {

        char * aux     = malloc(strlen(pt)*sizeof(char));
        char * type    = malloc(strlen(pt)*sizeof(char));
        char * subtype = malloc(strlen(pt)*sizeof(char));
        strcpy(aux, pt);   
        if(check_mime_format(aux, &type, &subtype) != 1){
            return -1; 
        }
        pt = strtok(NULL, ",");
    }

   return 0;
}


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


int validate_parameters(char * next_option, char * parameter) {
    if (strcmp(next_option, "-e") == 0) {
        if (validate_error_file(parameter) != 0){
            printf("Invalid error file\n");
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
    }

    return 0;
}

int validate_options(const int argc, char ** argv) {
    int arg;
    int option;
    char * next_option = NULL;
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
    printf("No errors found on input\n");
    return 0;
}


int parse_input(const int argc, char ** argv) {
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

    if (validate_options(argc, argv) < 0) {
        print_usage();
        return -1;
    }

    return 0;
}

void initialize_values() {
    parameters                                  = malloc(sizeof(*parameters));
    parameters->port                            = 1110;
    parameters->error_file                      = "/dev/null";
    parameters->management_address              = "127.0.0.1";
    parameters->management_port                 = 9090;
    parameters->listen_address                  = "0.0.0.0";
    parameters->replacement_msg                 = "Parte reemplazada.";
    parameters->filtered_media_types            = filter_list_init(); 
    parameters->origin_port                     = 110;
    parameters->filter_command                  = malloc(sizeof(*e_transformation));
    parameters->filter_command->switch_program  = false; //tiene que estar seteado en false
    parameters->filter_command->program_name    = (unsigned char *) "cat"; //tiene que estar seteado en cat
    parameters->version                         = "1.0";
}

options set_options_values(const int argc, char ** argv) {
    int c;

    parameters->origin_server  = argv[argc-1];
    optind                     = 1;

    while ((c = getopt(argc, argv, "e:l:L:m:M:o:p:P:t:")) != -1) {
        switch (c) {
            case 'e':
                parameters->error_file = optarg;
                break;
            case 'l':
                parameters->listen_address = optarg;
                break;
            case 'L':
                parameters->management_address = optarg;
                break;
            case 'm':
                parameters->replacement_msg = optarg;
                break;
            case 'M':
                ;
                char* pt;
                pt = strtok (optarg,",");
                    while (pt != NULL) {
                        char * type     = malloc(strlen(pt) * sizeof(char));
                        char * subtype  = malloc(strlen(pt) * sizeof(char));
                        check_mime_format(pt, &type, &subtype);
                        forbid_new(type, subtype, parameters->filtered_media_types);
                        pt = strtok (NULL, ",");
                    }
                break;
            case 'o':
                parameters->management_port = (uint16_t) atoi(optarg);
                break;
            case 'p':
                parameters->port = (uint16_t) atoi(optarg);
                break;
            case 'P':
                parameters->origin_port = (uint16_t) atoi(optarg);
                break;
            case 't':
                parameters->filter_command->program_name = (unsigned char *) optarg;
                break;
            default: /* If validate_option works correctly, it won't enter here */
                break;
        }
    }

    return parameters;
}
