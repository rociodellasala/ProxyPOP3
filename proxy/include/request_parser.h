#ifndef PROXYPOP3_REQUEST_PARSER_H
#define PROXYPOP3_REQUEST_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#include <netinet/in.h>

#include "buffer.h"
#include "request.h"

enum request_state {
    request_cmd,
    request_param,

    // apartir de aca están done
    request_done,

   // y apartir de aca son considerado con error
    request_error,
    request_error_cmd_too_long,
    request_error_param_too_long,
};

#define CMD_SIZE    4
#define PARAM_SIZE  40

struct request_parser {
    struct pop3_request  *request;
    enum request_state   state;

    uint8_t            i, j;
    // TODO podria usar buffer.c
    char                cmd_buffer[CMD_SIZE];
    char                param_buffer[PARAM_SIZE];
};

/** inicializa el parser */
void request_parser_init (struct request_parser *);

/** entrega un byte al parser. retorna true si se llego al final  */
enum request_state request_parser_feed (struct request_parser *, const uint8_t);

/**
 * por cada elemento del buffer llama a `request_parser_feed' hasta que
 * el parseo se encuentra completo o se requieren mas bytes.
 *
 * @param errored parametro de salida. si es diferente de NULL se deja dicho
 *   si el parsing se debió a una condición de error
 */
enum request_state request_consume(buffer *, struct request_parser *, bool *);

/**
 * Permite distinguir a quien usa request_parser_feed si debe seguir
 * enviando caracters o no. 
 *
 * En caso de haber terminado permite tambien saber si se debe a un error
 */
bool request_is_done(enum request_state, bool *);

void request_close(struct request_parser *);

/**
 * serializa la request en buff
 *
 * Retorna la cantidad de bytes ocupados del buffer o -1 si no había
 * espacio suficiente.
 */
extern int request_marshall(struct pop3_request *, buffer *);


#endif //PROXYPOP3_REQUEST_PARSER_H
