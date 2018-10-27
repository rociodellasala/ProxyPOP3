#ifndef PROXYPOP3_CONFIGURATIONCLIENT_H
#define PROXYPOP3_CONFIGURATIONCLIENT_H

#include "input_parser.h"
#include "utils.h"
#include <stdbool.h>

#define MAX_BUFFER 1024
#define MAX_READ MAX_BUFFER
#define SPACE ' '
#define NEWLINE '\n'

enum cmd {
    A           = '1',
    SET_T       = '2',
    GET_T       = '3',
    SWITCH_T    = '4',
    GET_ME      = '5',
    GET_MI      = '6',
    ALLOW_MI    = '7',
    FORBID_MI   = '8',
    Q           = '9',
    HELP        = '0',
};

typedef enum admin_status {
    ST_AUTH     = 0,
    ST_TRANS    = 1,
} admin_status;

typedef enum cmd_status {
    BAD_SINTAXIS    = 0,
    INEXISTENT_CMD  = 1,
    HELP_CMD = 2,
    WELL_WRITTEN = 4,
} cmd_status;

extern file_descriptor socket_fd;

void communicate_with_proxy();

cmd_status authenticate(enum cmd, char *, bool *);
cmd_status transaction(enum cmd, char *, bool *);

#endif //PROXYPOP3_CONFIGURATIONCLIENT_H
