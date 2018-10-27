#ifndef PROXYPOP3_SERIALIZER_H
#define PROXYPOP3_SERIALIZER_H

#include "request.h"

/* Serializa cada campo del request, es decir transforma todos los campos y
 * los junta en un char * que sera el que se enviará a través del socket */
unsigned char * serialize_request(unsigned char *, request *);

#endif //PROXYPOP3_SERIALIZER_H
