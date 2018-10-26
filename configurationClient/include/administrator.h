#ifndef PROXYPOP3_CONFIGURATIONCLIENT_H
#define PROXYPOP3_CONFIGURATIONCLIENT_H

#include "input_parser.h"
#include "utils.h"

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

void communicate_with_proxy(file_descriptor);

#endif //PROXYPOP3_CONFIGURATIONCLIENT_H
