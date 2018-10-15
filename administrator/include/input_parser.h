#ifndef PROXYPOP3_OPTIONSPARSER_H
#define PROXYPOP3_OPTIONSPARSER_H

#include "administrator.h"

/* Typedefs */
typedef struct {
    char * management_address;              // -L
    int management_port;                    // -o
} options;

/* Functions */
int parse_input(int, char **);
int validate_options(int, char **);
int validate_parameters(char *, char *);
int validate_address(char *);
int validate_port(char *);
void print_usage();
options initialize_values(options);
options set_options_values(options, int, char **);

#endif //PROXYPOP3_OPTIONSPARSER_H
