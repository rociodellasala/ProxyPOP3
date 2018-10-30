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

    request_done,

    request_error_inexistent_cmd,
    request_error_cmd_too_long,
    request_error_param_too_long,
};

#define CMD_SIZE    4
#define PARAM_SIZE  40

struct request_parser {
    struct pop3_request *   request;
    enum request_state      state;

    uint8_t                 i, j;
    char                    cmd_buffer[CMD_SIZE];
    char                    param_buffer[PARAM_SIZE];
};

void request_parser_init (struct request_parser *);

enum request_state request_parser_feed (struct request_parser *, const uint8_t);

enum request_state request_consume(buffer *, struct request_parser *, bool *);

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
