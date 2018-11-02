#ifndef PROXYPOP3_ADMIN_H
#define PROXYPOP3_ADMIN_H

#include "selector.h"
#include "buffer.h"
#include "admin_request.h"
#include "admin_response.h"
#include "utils.h"

enum b_cmd {
    A_CMD                   = 0X01,
    SET_T_CMD               = 0X02,
    GET_T_CMD               = 0X03,
    SWITCH_T_CMD            = 0X04,
    GET_ME_CMD              = 0X05,
    GET_MI_CMD              = 0X06,
    ALLOW_MI_CMD            = 0X07,
    FORBID_MI_CMD           = 0X08,
    Q_CMD                   = 0X09,
};

enum admin_status {
    ST_EHLO                 = 0,
    ST_CONNECTED            = 1,
};

enum parse_req_status {
    REQ_PARSE_OK            = 0,
    INCORRECT_PASS          = 1,
    COULD_NOT_READ_REQUEST  = 2,
    INCORRECT_METRIC        = 3,
    VERSION_UNSOPPORTED     = 4,
};

enum parse_resp_status {
    RESP_PARSE_OK           = 0,
    COULD_NOT_SEND_WELCOME  = 1,
    COULD_NOT_SEND_RESPONSE = 2,
};

struct admin {
    struct sockaddr_storage admin_addr;
    file_descriptor         fd;

    enum admin_status       a_status;

    struct request_admin *  current_request;
    enum parse_req_status   req_status;

    enum parse_resp_status  resp_status;
    size_t                  resp_length;
    char *                  resp_data;

    bool                    quit;
} admin;

/*
 * Handlers del administrador
 */
void admin_read(struct selector_key *);
void admin_write(struct selector_key *);
void admin_close(struct selector_key *);

/*
 * Acepta una nueva conexion (administrador) y crea un administrador
 * registrandolo en el selector
 */
void admin_accept_connection(struct selector_key *);

#endif  //PROXYPOP3_ADMIN_H
