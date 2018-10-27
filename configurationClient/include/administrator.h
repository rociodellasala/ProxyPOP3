#ifndef PROXYPOP3_CONFIGURATIONCLIENT_H
#define PROXYPOP3_CONFIGURATIONCLIENT_H

#include "input_parser.h"
#include "utils.h"
#include <stdbool.h>

#define SPACE ' '
#define NEWLINE '\n'

typedef enum {
    HELP        = '0',
    A           = '1',
    SET_T       = '2',
    GET_T       = '3',
    SWITCH_T    = '4',
    GET_ME      = '5',
    GET_MI      = '6',
    ALLOW_MI    = '7',
    FORBID_MI   = '8',
    Q           = '9',
} cmd;

typedef enum admin_status {
    ST_AUTH     = 0,
    ST_TRANS    = 1,
} admin_status;

typedef enum cmd_status {
    BAD_SINTAXIS    = 0,
    INEXISTENT_CMD  = 1,
    HELP_CMD        = 2,
    PARAM_TOO_LONG  = 3,
    WELL_WRITTEN    = 4,
} cmd_status;

extern file_descriptor socket_fd;

void communicate_with_proxy();

cmd_status authenticate(cmd, const char *, bool *);
cmd_status transaction(cmd, const char *, bool *);

#endif //PROXYPOP3_CONFIGURATIONCLIENT_H
