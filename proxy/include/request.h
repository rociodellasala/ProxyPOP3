#ifndef PROXYPOP3_REQUEST_H
#define PROXYPOP3_REQUEST_H

#include "response.h"

#define CMD_SIZE 12

enum pop3_cmd {
    error = -1,

    /* valid in the AUTHORIZATION state */
    user,
    pass,
    apop,  	// unsupported

    /* valid in the TRANSACTION state */
    stat,
    list,
    retr,
    dele,
    noop,
    rset,
    top,    // unsupported
    uidl,   // unsupported

    /* other */
    quit,
};

/* Puntero a funcion usado cuando se ejecutan los comandos */
typedef void (* command_fn)(struct pop3_request *);

struct pop3_request_cmd {
    const enum pop3_cmd 	id;
    const char *		    name;
    command_fn              fn;
};

struct pop3_request {
    const struct pop3_request_cmd * cmd;
    char *                          args;
    enum pop3_response_status       status;
};


/* Traduce un string a struct cmd */
const struct pop3_request_cmd * parse_cmd(const char *);


#endif //PROXYPOP3_REQUEST_H
