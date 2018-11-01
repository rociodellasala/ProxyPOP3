#ifndef PROXYPOP3_RESPONSE_PARSER_H
#define PROXYPOP3_RESPONSE_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#include <netinet/in.h>

#include "buffer.h"
#include "client_response.h"
#include "client_request.h"
#include "parser.h"

#define STATUS_SIZE         4

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

    uint8_t               i, j;

    char                  status_buffer[STATUS_SIZE];

    bool                  first_line_done;
    struct parser         *pop3_multi_parser;

    char                  *capa_response;
    size_t                capa_size;
};

/** inicializa el parser */
void response_parser_init (struct response_parser *p);

/** entrega un byte al parser. retorna true si se llego al final  */
enum response_state response_parser_feed (struct response_parser *p, uint8_t c);

enum response_state response_consume(buffer *b, buffer *wb, struct response_parser *p, bool *errored);

bool response_is_done(enum response_state st, bool *errored);



#endif //PROXYPOP3_RESPONSE_PARSER_H
