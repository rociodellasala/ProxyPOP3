#ifndef PROXYPOP3_OPTIONSPARSER_H
#define PROXYPOP3_OPTIONSPARSER_H

struct options{
    char *  management_address;              // -L
    int     management_port;                 // -o
};

typedef struct options * options;

extern options parameters;

int parse_input(int, char **);
void initialize_values();
void set_options_values(int, char **);

#endif //PROXYPOP3_OPTIONSPARSER_H
