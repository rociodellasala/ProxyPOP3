#ifndef PROXYPOP3_OPTIONSPARSER_H
#define PROXYPOP3_OPTIONSPARSER_H

#include <stdint.h>

struct options {
    char *      server_address;
    uint16_t    server_port;
};

typedef struct options * options;

extern options clt_parameters;

int parse_input(int, char **);
void initialize_values();
void set_options_values(int, char **);

#endif //PROXYPOP3_OPTIONSPARSER_H
