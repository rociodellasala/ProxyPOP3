#ifndef PROXYPOP3_RESPONSE_H
#define PROXYPOP3_RESPONSE_H

typedef struct {
    unsigned char   version;
    unsigned char   status;
    unsigned int    length;
    unsigned char * data;
} response;

typedef enum {
   ERROR            = 0,
   OK               = 1,
   ERROR_RECEIVING  = 2,
   NOT_SEND         = 3,     
} response_status;


#endif //PROXYPOP3_RESPONSE_H
