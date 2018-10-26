#ifndef PROXYPOP3_DESERIALIZER_H
#define PROXYPOP3_DESERIALIZER_H

#include "request_admin.h"

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

unsigned char * deserialize_request(unsigned char *, request_admin *);

#endif //PROXYPOP3_DESERIALIZER_H
