#include <malloc.h>

#include "include/response.h"

unsigned char * deserialize_int(unsigned char * buffer, unsigned int * value) {
    *value = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
    
    return buffer + 4;
}

unsigned char * deserialize_char(unsigned char * buffer, unsigned char * value) {
    *value = buffer[0];
    
    return buffer + 1;
}

unsigned char * deserialize_string(unsigned char * buffer, unsigned char * str, const unsigned int length) {
    unsigned int i = 0;
    
    do {
        buffer = deserialize_char(buffer, str);
        str++;
    } while (++i != length);
    
    return buffer;
}

unsigned char * deserialize_response(unsigned char * buffer, struct response * response) {
    buffer = deserialize_char(buffer, &response->version);
    buffer = deserialize_char(buffer, &response->status);
    buffer = deserialize_int(buffer, &response->length);

    if (response->length > 0) {
        response->data = malloc((response->length + 1) * sizeof(unsigned char *));

        if (response->data == NULL) {
            return NULL;
        }

        buffer = deserialize_string(buffer, response->data, response->length);
        response->data[response->length] = '\0';
    }

    return buffer;
}
