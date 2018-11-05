#ifndef PROXYPOP3_SERIALIZER_H
#define PROXYPOP3_SERIALIZER_H

#include "client_request.h"

unsigned char * serialize_request(unsigned char *, struct request *);

#endif //PROXYPOP3_SERIALIZER_H
