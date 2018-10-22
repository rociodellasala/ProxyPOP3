#ifndef MIME_TYPE_H_
#define MIME_TYPE_H_

/**
 * mime_type.c - tokenizador de MIME types
 *
 */
#include "parser.h"

/** tipo de eventos de un MIME type */
//struct parser;
enum mime_type_event_type {
    /* caracter del tipo de un MIME type. payload: caracter. */
    MIME_TYPE_TYPE,

    /* el nombre del tipo está completo. payload: '/'. */
    MIME_TYPE_TYPE_END,

    /* caracter del subtipo de un MIME type. payload: caracter. */
    MIME_TYPE_SUBTYPE,

    /* el subtipo esta completo. payload: ';'.*/
    MIME_PARAMETER_START,

    MIME_PARAMETER,

    MIME_BOUNDARY_END,

    MIME_FRONTIER_START,

    MIME_FRONTIER,

    MIME_FRONTIER_END,

    /* se recibió un caracter que no se esperaba */
    MIME_TYPE_UNEXPECTED,
};

/** la definición del parser */
const struct parser_definition * mime_type_parser(void);

const char *
mime_type_event(enum mime_type_event_type type);


#endif