#include <malloc.h>
#include "include/response_admin.h"
#include "include/request_admin.h"

unsigned char * deserialize_int(unsigned char * buffer, unsigned int * value) {
    printf("buffer %s\n", buffer);
    *value = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
    return buffer + 4;
}

unsigned char * deserialize_char(unsigned char * buffer, unsigned char * value) {
    *value = buffer[0];
    return buffer + 1;
}

unsigned char * deserialize_string(unsigned char * buffer, unsigned char * str, unsigned int length) {
    int i = 0;
    do {
        buffer = deserialize_char(buffer, str);
        str++;
    } while (++i != length);
    return buffer;
}

unsigned char * deserialize_request(unsigned char * buffer, request * request) {

    /** deserialization of type */
    buffer = deserialize_char(buffer, &request->version);
    /** buffer size deserialization */
    buffer = deserialize_char(buffer, &request->cmd);
    /** buffer size deserialization */
    buffer = deserialize_int(buffer, &request->length);

    if (request->length > 0) {
        request->data = malloc((size_t)request->length);
        if (request->data == NULL)
            return NULL;
        buffer = deserialize_string(buffer, request->data, request->length);
    }
    
    return buffer;
}
