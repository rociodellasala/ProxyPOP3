#ifndef PROXYPOP3_REQUEST_H
#define PROXYPOP3_REQUEST_H

/* Typedefs */

#define VERSION 1

enum req_cmd {
    AUTH            = 1,
    SET_TRANSF      = 2,
    GET_TRANSF      = 3,
    SWITCH_TRANSF   = 4,
    GET_METRIC      = 5,
    GET_MIME        = 6,
    ALLOW_MIME      = 7,
    FORBID_MIME     = 8,
    QUIT            = 9,
};

typedef struct {
    unsigned char   version;
    unsigned char    cmd;
    unsigned int    length;
    unsigned char * data;
}request_admin;


#endif //PROXYPOP3_REQUEST_H
