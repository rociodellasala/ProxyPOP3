#ifndef PROXYPOP3_ADMIN_H
#define PROXYPOP3_ADMIN_H

#include "selector.h"
#include "buffer.h"
#include "admin_request.h"
#include "admin_response.h"

typedef enum {
    A_CMD         = 0X01,
    SET_T_CMD     = 0X02,
    GET_T_CMD     = 0X03,
    SWITCH_T_CMD  = 0X04,
    GET_ME_CMD    = 0X05,
    GET_MI_CMD    = 0X06,
    ALLOW_MI_CMD  = 0X07,
    FORBID_MI_CMD = 0X08,
    Q_CMD         = 0X09,
} b_cmd;

typedef enum admin_status {
    ST_EHLO         = 0,
    ST_CONNECTED    = 1,
} admin_status;

typedef enum parse_req_status {
    REQ_PARSE_OK                = 0,
    INCORRECT_PASS              = 1,
    COULD_NOT_READ_REQUEST      = 2,
    INCORRECT_METRIC            = 3,
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
    size_t                        resp_length;
    char *                        resp_data;

    unsigned int                  quit;

};

void admin_read(struct selector_key *);
void admin_write(struct selector_key *);
void admin_close(struct selector_key *);
void admin_accept_connection(struct selector_key *);


#endif  //PROXYPOP3_ADMIN_H
