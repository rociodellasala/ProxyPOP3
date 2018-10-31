#ifndef POP_MULTI_bf9b63c724e54ba2d17af1709493f755a54975f3
#define POP_MULTI_bf9b63c724e54ba2d17af1709493f755a54975f3

#include "parser.h"

/**
 * pop3_multi.c - parser de una respuesta multilinea POP3.
 *
 * Se encarga del "byte-stuffing" y de detectar el final.
 */

/* Eventos del parser multilinea pop3 */
enum pop3_multi_type {
    // N bytes nuevos
    POP3_MULTI_BYTE,
    // hay que esperar, no podemos decidir
    POP3_MULTI_WAIT,
    // la respuesta está completa
    POP3_MULTI_FIN,
};

/* La definición del parser */
const struct parser_definition * pop3_multi_parser(void);

#endif
