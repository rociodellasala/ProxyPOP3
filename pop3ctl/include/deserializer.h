#ifndef PROXYPOP3_DESERIALIZER_H
#define PROXYPOP3_DESERIALIZER_H

#include "response.h"

/* todo: https://stackoverflow.com/questions/1653681/serialization-deserialization-of-a-struct-to-a-char-in-c */
/* Deserializa cada campo del response, es decir transforma el char * recibido al campo correspondiente */
unsigned char * deserialize_response(unsigned char *, struct response *);

#endif //PROXYPOP3_DESERIALIZER_H
