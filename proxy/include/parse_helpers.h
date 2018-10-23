#ifndef PROXYPOP3_PARSE_HELPERS_H
#define PROXYPOP3_PARSE_HELPERS_H

#include "buffer.h"

enum helper_errors{
    PARSE_OK         = 0,
    ERROR_MALLOC     = 1,
    ERROR_DISCONNECT = 2,
    ERROR_WRONGARGS  = 3,
};

// char ** parse_text(buffer * b, char ** cmd, int args, int * args_found, bool * cmd_found);
char ** sctp_parse_cmd(buffer *, struct admin *, int *, int *);

void free_cmd(char **, int);

void send_error(struct admin *, const char *);

void send_ok(struct admin *, const char *);

#endif //PROXYPOP3_PARSE_HELPERS_H
