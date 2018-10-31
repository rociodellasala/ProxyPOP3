#ifndef PROXYPOP3_RESPONSE_PARSER_H
#define PROXYPOP3_RESPONSE_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#include <netinet/in.h>

#include "buffer.h"
#include "client_response.h"
#include "request.h"
#include "parser.h"

#define STATUS_SIZE         4
#define MAX_RESPONSE_SIZE   512
#define DESCRIPTION_SIZE    (MAX_RESPONSE_SIZE - STATUS_SIZE - 2)

enum response_state {
    response_status_indicator,
    response_description,
    response_newline,
    response_mail,
    response_list,
    response_capa,

    // estado usado para respuestas multilinea que no requieren un manejo especial
            response_multiline,

    // apartir de aca están done
            response_done,

    // y apartir de aca son considerado con error
            response_error,
};

struct response_parser {
    struct pop3_request  *request;
    enum response_state   state;

    uint8_t               i, j, count;

    char                  status_buffer[STATUS_SIZE];
    char                  description_buffer[DESCRIPTION_SIZE];     // unused

    bool                  first_line_done;
    struct parser         *pop3_multi_parser;

    char                  *capa_response;
    size_t                capa_size;
};

/** inicializa el parser */
void
response_parser_init (struct response_parser *p);

/** entrega un byte al parser. retorna true si se llego al final  */
enum response_state
response_parser_feed (struct response_parser *p, uint8_t c);

/**
 * por cada elemento del buffer llama a `response_parser_feed' hasta que
 * el parseo se encuentra completo o se requieren mas bytes.
 *
 * @param errored parametro de salida. si es diferente de NULL se deja dicho
 *   si el parsing se debió a una condición de error
 */
enum response_state
response_consume(buffer *b, buffer *wb, struct response_parser *p, bool *errored);

/**
 * Permite distinguir a quien usa response_parser_feed si debe seguir
 * enviando caracters o no.
 *
 * En caso de haber terminado permite tambien saber si se debe a un error
 */
bool
response_is_done(enum response_state st, bool *errored);

void
response_parser_close(struct response_parser *p);


#endif //PROXYPOP3_RESPONSE_PARSER_H
