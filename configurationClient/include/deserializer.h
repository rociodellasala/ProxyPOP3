#ifndef PROXYPOP3_DESERIALIZER_H
#define PROXYPOP3_DESERIALIZER_H

#include "response.h"

/* Functions */

/**
 * DESERIALIZERS
 * Deserializes the corresponding type so that it can be re built after reading
 * from the socket, ensuring a good reception.
 * @param buffer containing what needs to be deserialized
 * @param value where it saves the deserialized value
 * @return the buffer's direction after serializing, so that the message can
 * continue to deserialize
 */

unsigned char * deserialize_int(unsigned char *, unsigned int *);
unsigned char * deserialize_char(unsigned char *, unsigned char * );
unsigned char * deserialize_string(unsigned char *, unsigned char *, unsigned int);
unsigned char * deserialize_response(unsigned char *, response *);

#endif //PROXYPOP3_DESERIALIZER_H