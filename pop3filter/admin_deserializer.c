#include <malloc.h>
#include "include/admin_request.h"

unsigned char * deserialize_int(unsigned char * buffer, unsigned int * value) {
    *value = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
    return buffer + 4;
}

unsigned char * deserialize_char(unsigned char * buffer, unsigned char * value) {
    *value = buffer[0];
    return buffer + 1;
}

unsigned char * deserialize_string(unsigned char * buffer, unsigned char * str, unsigned int length) {
    unsigned int i = 0;
    do {
        buffer = deserialize_char(buffer, str);
        str++;
    } while (++i != length);
    return buffer;
}

unsigned char * deserialize_request(unsigned char * buffer, request_admin * request) {
    /** deserialization of type */
    buffer = deserialize_char(buffer, &request->version);

    /** buffer size deserialization */
    buffer = deserialize_char(buffer, &request->cmd);

    /** buffer size deserialization */
    buffer = deserialize_int(buffer, &request->length);

    if (request->length > 0) {
        request->data = malloc((request->length) * sizeof(unsigned char *));

        if (request->data == NULL) {
            return NULL;
        }

        buffer = deserialize_string(buffer, request->data, request->length);
        
    }
    
    return buffer;
}
