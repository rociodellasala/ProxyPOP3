#ifndef PROXYPOP3_REQUEST_H
#define PROXYPOP3_REQUEST_H

#define VERSION 1

typedef struct {
    unsigned char   version;
    unsigned char    cmd;
    unsigned int    length;
    unsigned char * data;
}request_admin;


#endif //PROXYPOP3_REQUEST_H
