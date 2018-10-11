#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "include/optionsParser.h"

/**
 * Prints help for the user
 */
void print_help() {
    /* TODO: Help utility */
    printf(" --- HELP ---\n");
    print_usage();
}

/**
 * Prints the proxy version
 */
void print_version() {
    printf("Version: POP3 Proxy 1.0\n");
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


/* return 0 if IP string is valid, else return -1 */
int isValidIp(char * ip_str) {
    int i, num, dots = 0;
    char *ptr;

    if (ip_str == NULL)
        return -1;

    ptr = strtok(ip_str, DELIM);

    if (ptr == NULL)
        return -1;

    while (ptr) {

        /* after parsing string, it must contain only digits */
        if (!valid_digit(ptr))
            return -1;

        num = atoi(ptr);

        /* check for valid IP */
        if (num >= 0 && num <= 255) {
            /* parse remaining string */
            ptr = strtok(NULL, DELIM);
            if (ptr != NULL)
                ++dots;
        } else
            return -1;
    }

    /* valid IP string must contain 3 dots */
    if (dots != 3)
        return -1;
    return 0;
}


/* return 0 if string contain only digits, else return -1 */
int valid_digit(char *ip_str) {
    while (*ip_str) {
        if (*ip_str >= '0' && *ip_str <= '9')
            ++ip_str;
        else
            return -1;
    }
    return 0;
}

int validate_origin_server_argument(char * origin_server) {
    // TODO: Validate origin server argument

    if(origin_server == 'localhost'){
        return 0;
    }
    else if(isValidIp(origin_server)){
        printf("Valid IP!\n");
        return 0;
    }

    //TODO: VALIDATE IPV6 ??

    return -1;
}


/* /* TODO: Fix problems
 * CASOS EN LOS QUE NO ESTA FUNCIONANDO :
 * ./exe -m -p hola (es decir, el primer argumento lo pongo mal y el segundo bien) porque me toma que
 * -p es el argumento de -m */
int validate_options(int argc, char ** argv) {
    int arg;
    int option;
    int i;
    int size            = argc-1;
    char ** options     = (char**) malloc(size * sizeof(char*));;

    for(arg = 0; arg < size; arg++){
        options[arg] = malloc(sizeof(char) * strlen(argv[arg]) + 1);
        strcpy(options[arg], argv[arg]);
    }

    opterr = 0;
    char* aux = argv[optind];
    if(!(strcmp("-e",aux) == 0 ||strcmp("-l",aux) == 0  || strcmp("-L",aux) == 0  || strcmp("-m",aux) == 0  || strcmp("-M",aux) == 0 
        || strcmp("-o",aux) == 0  || strcmp("-p",aux) == 0  || strcmp("-P",aux) == 0  || strcmp("-t",aux) == 0 )){
        printf("'%s' is an invalid option. Try with -e, -l, -L, -m, -M, -o, -p, -P or -t\n", aux);
    }

    for(arg = 1; arg < size; arg++) {
        /* http://man7.org/linux/man-pages/man3/getopt.3.html */
        option = getopt(size, options, ":e:l:L:m:M:o:P:p:t:");
       // printf("ok el coso es %s\n", argv[optind-1]);
        if(option == ':'){
            printf("Missing option argument for '%s'\n", argv[optind-1]);
            return -1;
        } else if (option == '?'){
            printf("Invalid option: '%s'\n", argv[optind-1]);
            return -1;
        }
    }

    for(i = 0; i < size ; i++)
        free(options[i]);

    free(options);

    /* For debug, remember to take it out ! */
    printf("No errors found on input ! \n");
}

options set_options_values(options opt, int argc, char ** argv) {
    int c;

    opt.origin_server   = argv[argc-1];
    optind              = 1;

    while((c = getopt(argc, argv, "e:l:L:m:M:o:p:P:t:")) != -1){
        switch(c){
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

int parse_input(int argc, char ** argv) {

    if(argc < 2){
        printf("Program execution requires at least one parameter \n");
        print_usage();
        return -1;
    }

    if(strcmp(argv[1], "-v") == 0) {
        print_version();
        exit(0);
    }

    if(strcmp(argv[1], "-h") == 0) {
        print_help();
        exit(0);
    }

    if(validate_options(argc, argv) < 0 || validate_origin_server_argument(argv[argc-1]) < 0) {
        print_usage();
        return -1;
    }

    return 0;
}

