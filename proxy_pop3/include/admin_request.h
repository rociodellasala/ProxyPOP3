#ifndef PROXYPOP3_REQUEST_H
#define PROXYPOP3_REQUEST_H

struct request_admin{
    unsigned char   version;
    unsigned char   cmd;
    unsigned int    length;
    unsigned char * data;
};


#endif //PROXYPOP3_REQUEST_H
