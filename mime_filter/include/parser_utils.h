#ifndef PROXYPOP3_PARSERUTILS_H
#define PROXYPOP3_PARSERUTILS_H

/*
 * parser_utils.c -- factory de ciertos parsers típicos
 *
 * Provee parsers reusables, como por ejemplo para verificar que
 * un string es igual a otro de forma case insensitive.
 */
#include "parser_creator.h"

enum string_cmp_event_type {
    /** hay posibilidades de que el string sea igual */
    STRING_CMP_EQ,
    /** NO hay posibilidades de que el string sea igual */
    STRING_CMP_NEQ,
};

/*
 * Crea un parser que verifica que los caracteres recibidos forment el texto
 * descripto por `s'.
 *
 * Si se recibe el evento `STRING_CMP_NEQ' el texto entrado no matchea.
 */
struct parser_definition parser_utils_strcmpi(const char *);

/*
 * Crea un parser que verifica que los caracteres recibidos formen alguno
 * de los Strings en la lista de filtros
 */

/**
 * libera recursos asociado a una llamada de `parser_utils_strcmpi'
 */
void parser_utils_strcmpi_destroy(const struct parser_definition *);


#endif
