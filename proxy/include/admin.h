#ifndef PROXYPOP3_ADMIN_H
#define PROXYPOP3_ADMIN_H

#include "selector.h"
#include "buffer.h"
#include "request_admin.h"
#include "response_admin.h"

#define BUFFER_SIZE 1024

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
};

typedef enum admin_status {
    ST_EHLO     = 0,
    ST_AUTH     = 1,
    ST_TRANS    = 2,
} admin_status;

typedef enum parse_req_status {
    REQ_PARSE_OK                = 0,
    INCORRECT_PASS              = 1,
    INCORRECT_COMMAND_STATUS    = 2,
    COULD_NOT_READ_REQUEST      = 3,
    INCORRECT_METRIC            = 4,
} parse_req_status;

typedef enum parse_resp_status {
    RESP_PARSE_OK               = 0,
    COULD_NOT_SEND_WELCOME      = 1,
    COULD_NOT_SEND_RESPONSE     = 2,
} parse_resp_status;

struct admin {
    struct sockaddr_storage       admin_addr;
    int                           fd;

    admin_status                  a_status;

    request_admin *               current_request;
    parse_req_status              req_status;

    parse_resp_status             resp_status;
    unsigned int                  resp_length;
    unsigned char *               resp_data;

    unsigned int                  quit;

};

void admin_read(struct selector_key * key);
void admin_write(struct selector_key * key);
void admin_close(struct selector_key * key);

void admin_accept_connection(struct selector_key *);


#endif  //PROXYPOP3_ADMIN_H
