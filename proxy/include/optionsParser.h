#ifndef PROXYPOP3_OPTIONSPARSER_H
#define PROXYPOP3_OPTIONSPARSER_H

#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DELIM "."

/* Typedefs */
typedef struct {
    int port;                      // -p
    char * error_file;             // -e
    char * listen_address;         // -l
    char * management_address;     // -L
    int management_port;           // -o
    char * replacement_msg;        // -m
    char * filtered_media_types;   // -M
    char * origin_server;          // This is the argument origin_server, it's not an option
    int origin_port;               // -P
    char *filter_command;          // -t
} options;

/* Functions */
void print_help();

void print_version();

void print_usage();

int validate_origin_server_argument(char *);

int validate_options(int, char **);

options set_options_values(options, int, char **);

options initialize_values(options);

int parse_input(int, char **);

int validate_port(char *);

int validate_transformation(char *);

int validate_address(char *);

int validate_message(char *);

int validate_media_type(char *);

int validate_error_file(char *);

#endif //PROXYPOP3_OPTIONSPARSER_H
