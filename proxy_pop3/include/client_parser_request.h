#ifndef PROXYPOP3_REQUEST_PARSER_H
#define PROXYPOP3_REQUEST_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "buffer.h"
#include "client_request.h"

#define MAX_QUANTITY_PARAMS             2

typedef enum request_state {
    request_cmd,
    request_parameter,
    request_newline,
    request_done,
    // a partir de aca es un error
    request_error_inexistent_cmd,
    request_error_cmd_too_long,
    request_error_param_too_long,
    request_error_too_many_params,
} request_state;

struct request_parser {
    struct pop3_request *   request;
    enum request_state      state;
    uint8_t                 i, j;
    char                    cmd_buffer[MAX_CMD_SIZE];
    char                    param_buffer[MAX_QUANTITY_PARAMS][MAX_PARAM_SIZE];
    int                     params;
};

void request_parser_reset (struct request_parser *);
enum request_state request_consume(buffer *, struct request_parser *, bool *);
bool request_is_done(enum request_state, bool *);

#endif //PROXYPOP3_REQUEST_PARSER_H
