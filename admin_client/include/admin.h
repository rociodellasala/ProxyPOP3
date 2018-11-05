#ifndef PROXYPOP3_CONFIGURATIONCLIENT_H
#define PROXYPOP3_CONFIGURATIONCLIENT_H

#include <stdbool.h>

#include "input_parser.h"
#include "utils.h"

typedef enum {
    HELP            = '0',
    A               = '1',
    SET_T           = '2',
    GET_T           = '3',
    SWITCH_T        = '4',
    GET_ME          = '5',
    GET_MI          = '6',
    ALLOW_MI        = '7',
    FORBID_MI       = '8',
    Q               = '9',
} cmd;

typedef enum {
    A_CMD           = 0X01,
    SET_T_CMD       = 0X02,
    GET_T_CMD       = 0X03,
    SWITCH_T_CMD    = 0X04,
    GET_ME_CMD      = 0X05,
    GET_MI_CMD      = 0X06,
    ALLOW_MI_CMD    = 0X07,
    FORBID_MI_CMD   = 0X08,
    Q_CMD           = 0X09,
} b_cmd;

typedef enum admin_status {
    ST_AUTH         = 0,
    ST_TRANS        = 1,
} admin_status;

typedef enum cmd_status {
    BAD_SINTAXIS    = 0,
    INEXISTENT_CMD  = 1,
    HELP_CMD        = 2,
    PARAM_TOO_LONG  = 3,
    PARAM_TOO_SHORT = 4,
    WELL_WRITTEN    = 5,
} cmd_status;

extern file_descriptor socket_fd;

void communicate_with_proxy();
cmd_status authenticate(cmd, const char *, bool *);
cmd_status transaction(cmd, const char *, bool *);

#endif //PROXYPOP3_CONFIGURATIONCLIENT_H
