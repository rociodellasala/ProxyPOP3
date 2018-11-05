#ifndef PROXYPOP3_RESPONSE_H
#define PROXYPOP3_RESPONSE_H

struct response_admin {
    unsigned char   version;
    unsigned char   status;
    unsigned int    length;
    unsigned char * data;
};

#endif //PROXYPOP3_RESPONSE_H
