#ifndef PROXYPOP3_INPUT_PARSER_H
#define PROXYPOP3_INPUT_PARSER_H

#include <stdbool.h>
#include <stdint.h>
#include "filter_list.h"

#define DELIM "."
#define BACKLOG 10

struct extern_transformation {
    bool switch_program;
    unsigned char * program_name;
};

typedef struct extern_transformation * transformation;

extern transformation e_transformation;

struct options {
    uint16_t            port;                 // -p
    char *              error_file;           // -e
    char *              listen_address;       // -l
    char *              management_address;   // -L
    uint16_t            management_port;      // -o
    char *              replacement_msg;      // -m
    struct filter_list* filtered_media_types; // -M
    char *              origin_server;        // This is the argument origin_server, it's not an option
    uint16_t            origin_port;          // -P
    transformation      filter_command;       // -t
    char *              version;
};

typedef struct options * options;

extern options parameters;

/* Chequea que no se haya llamado a -v o -h. Llama a las funciones
 * para validar el formato de las opciones y de los argumentos */
int parse_input(int, char **);

/* Inicializa en la estructura de parametros los valores default */
void initialize_values();

/* Setea los parametros segun lo ingresado como opcion */
options set_options_values(int, char **);

#endif //PROXYPOP3_INPUT_PARSER_H
