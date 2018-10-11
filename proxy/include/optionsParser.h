#ifndef PROXYPOP3_OPTIONSPARSER_H
#define PROXYPOP3_OPTIONSPARSER_H

#define DELIM "."

/* Try to check types of arguments */
typedef struct {
    int port;                      // -p
    char * error_file;             // -e
    char * listen_address;         // -l
    char * management_address;     // -L
    int management_port;           // -o
    char * replacement_msg;        // -m
    char * filtered_media_types;   // -M
    char * origin_server;          // Este es el argumento, no es opcion
    int origin_port;               // -P
    char *filter_command;          // -t
} options;

void print_help();

void print_version();

void print_usage();

int validate_origin_server_argument(char *);

int validate_options(int, char **);

options set_options_values(options, int, char **);

options initialize_values(options);

int parse_input(int, char **);

#endif //PROXYPOP3_OPTIONSPARSER_H
