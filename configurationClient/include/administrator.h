#ifndef PROXYPOP3_CONFIGURATIONCLIENT_H
#define PROXYPOP3_CONFIGURATIONCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "input_parser.h"
#include "response.h"

#define MAX_BUFFER 1024
#define MAX_READ 1024
#define SPACE ' '
#define NEWLINE '\n'

/* Typedefs */
typedef int file_descriptor;

/* Enums */

enum cmd {
    A = '1',
    SET_T = '2',
    GET_T = '3',
    SWITCH_T = '4',
    GET_ME = '5',
    GET_MI = '6',
    ALLOW_MI = '7',
    FORBID_MI = '8',
    Q = '9',
    HELP = '0',
};

/* Functions */
void communicate_with_proxy(file_descriptor);
file_descriptor initialize_sctp_socket(options);
void assemble_req(int, int, unsigned char *, int *, file_descriptor, enum cmd);

#endif //PROXYPOP3_CONFIGURATIONCLIENT_H
