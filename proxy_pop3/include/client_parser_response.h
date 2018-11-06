#ifndef PROXYPOP3_RESPONSE_PARSER_H
#define PROXYPOP3_RESPONSE_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#include <netinet/in.h>

#include "buffer.h"
#include "client_response.h"
#include "client_request.h"
#include "parser.h"

#define STATUS_SIZE  4
#define BLOCK_SIZE  20

enum response_state {
    response_status,
    response_description,
    response_newline,
    response_get_mail,
    response_list,
    response_capa,
    response_multiline,
    response_done,
    response_error,
};

struct response_parser {
    struct pop3_request  *  request;
    enum response_state     state;

    uint8_t                 i, j;

    char                    status_buffer[STATUS_SIZE];

    bool                    first_line_consumed;
    struct parser *         pop3_multi_parser;

    char *                  capa_response;
    size_t                  capa_size;
};


void response_parser_init(struct response_parser *);

enum response_state response_consume(buffer *, buffer *, struct response_parser *, bool *);

bool response_is_done(enum response_state, bool *);

#endif //PROXYPOP3_RESPONSE_PARSER_H
