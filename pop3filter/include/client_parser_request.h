#ifndef PROXYPOP3_REQUEST_PARSER_H
#define PROXYPOP3_REQUEST_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "buffer.h"
#include "client_request.h"

#define MAX_CONCURRENT_INVALID_COMMANDS 5

typedef enum request_state {
    request_cmd,
    request_param,
    request_newline,

    // a partir de aca están done
    request_done,

    // y a partir de aca son considerado con error
    request_error_inexistent_cmd,
    request_error_cmd_too_long,
    request_error_param_too_long,
} request_state;

struct request_parser {
    struct pop3_request *   request;
    enum request_state      state;
    uint8_t                 i, j;
    char                    cmd_buffer[CMD_SIZE];
    char                    param_buffer[PARAM_SIZE];
};

void request_parser_reset (struct request_parser *);
enum request_state request_parser_feed (struct request_parser *, uint8_t);
enum request_state request_consume(buffer *, struct request_parser *, bool *);
bool request_is_done(enum request_state, bool *);

#endif //PROXYPOP3_REQUEST_PARSER_H
