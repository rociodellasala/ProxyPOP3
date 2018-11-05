#ifndef PROXYPOP3_DELIMITER_H
#define PROXYPOP3_DELIMITER_H

#include <stdint.h>

#define MAX_LENGTH 100

/* Llamamos delimiter al valor que le asignamos a un boundary. */
struct delimiter_st {
    char delimiter[MAX_LENGTH];
    uint8_t delimiter_size;
    struct parser_definition *delimiter_parser_def;
    struct parser_definition *delimiter_end_parser_def;
    struct parser *delimiter_parser;
    struct parser *delimiter_end_parser;
    
};

/* Inicializaciamos la estructura*/
struct delimiter_st * delimiter_init();

/* Crea el delimiter de cierre agregando los caracteres correspondientes */
void close_delimiter(struct delimiter_st *delimiter);

/* Reseteamos los parsers correspondientes */
void delimiter_reset(struct delimiter_st *delimiter);

/* Destruye los parsers correspondientes */
void delimiter_destroy(struct delimiter_st *delimiter);

/* Agregamos un caracter al delimiter*/
void extend(const uint8_t c, struct delimiter_st *delimiter);



#endif 
