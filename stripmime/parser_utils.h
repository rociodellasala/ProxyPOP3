#ifndef PARSER_UTILS_H_
#define PARSER_UTILS_H_

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
struct parser_definition
parser_utils_strcmpi(const char *s);

/*
 * Crea un parser que verifica que los caracteres recibidos formen alguno
 * de los Strings en la lista de filtros
 */
struct parser_definition
parser_utils_list_strcmpi(const char *filtered_list[]);

/**
 * libera recursos asociado a una llamada de `parser_utils_strcmpi'
 */
void
parser_utils_strcmpi_destroy(const struct parser_definition *p);

const char *
parser_utils_strcmpi_event(enum string_cmp_event_type type);

#endif
