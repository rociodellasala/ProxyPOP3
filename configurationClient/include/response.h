#ifndef PROXYPOP3_RESPONSE_H
#define PROXYPOP3_RESPONSE_H

typedef struct {
    unsigned char   version;
    unsigned char   status;
    unsigned int    length;
    unsigned char * data;
} response;

#endif //PROXYPOP3_RESPONSE_H
