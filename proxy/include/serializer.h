#ifndef PROXYPOP3_SERIALIZER_H
#define PROXYPOP3_SERIALIZER_H

#include "request_admin.h"
#include "response_admin.h"

/* Functions */

/**
* SERIALIZERS
* Serializes the corresponding type so that it can be sent through the socket,
* ensuring a good arrival.
* @param buffer with space to save what gets serialized
* @param value to be serialized
* @return the buffer's direction after serializing, so that the complete message
* gets serialized in contiguous memory
*/

unsigned char * serialize_response(unsigned char *, response_admin *);

#endif //PROXYPOP3_SERIALIZER_H
