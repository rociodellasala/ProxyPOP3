#ifndef PROXYPOP3_DESERIALIZER_H
#define PROXYPOP3_DESERIALIZER_H

#include "admin_request.h"

unsigned char * deserialize_request(unsigned char *, struct request_admin *);

#endif //PROXYPOP3_DESERIALIZER_H
