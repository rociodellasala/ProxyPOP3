#ifndef PROXYPOP3_MIMETYPE_H
#define PROXYPOP3_MIMETYPE_H

#include "parser_creator.h"

/* eventos en un mime type*/
enum mime_type_event_type {
    
    /* Caracter perteneciente al MIME type. payload: caracter. */
    MIME_TYPE_TYPE,

    /* Fin del MIME type. payload: '/'. */
    MIME_TYPE_TYPE_END,

    /* Caracter perteneciente al MIME subtype. payload: caracter. */
    MIME_TYPE_SUBTYPE,

    /* Fin del subtipo. payload: ';'.*/
    MIME_PARAMETER_START,

    /* Caracter perteneciente al parameter. payload: caracter. */
    MIME_PARAMETER,

    /* Fin del parameter. payload: '='. */
    MIME_BOUNDARY_END,

    /* Comienzo del valor asignado a boundary (delimiter). payload '"' */
    MIME_DELIMITER_START,

    /* Caracter perteneciente al delimiter. payload: caracter. */
    MIME_DELIMITER,

    /* Fin del valor asignado a boundary (delimiter). payload '"' */
    MIME_DELIMITER_END,

    /* Se recibió un caracter que no se esperaba */
    MIME_TYPE_UNEXPECTED,
};

const struct parser_definition * mime_type_parser(void);

#endif