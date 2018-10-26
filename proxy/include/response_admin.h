#ifndef PROXYPOP3_RESPONSE_H
#define PROXYPOP3_RESPONSE_H

typedef struct {
    unsigned char   version;
    unsigned char   status;
    unsigned int    length;
    char *data;
}response_admin;

#endif //PROXYPOP3_RESPONSE_H
