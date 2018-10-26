#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/request.h"

unsigned char * serialize_int(unsigned char * buffer, unsigned int value) {
    buffer[0] = (unsigned char)(value >> 24 & 0xFF);
    buffer[1] = (unsigned char)(value >> 16 & 0xFF);
    buffer[2] = (unsigned char)(value >> 8 & 0xFF);
    buffer[3] = (unsigned char)(value & 0xFF);
    return buffer + 4;
}

unsigned char * serialize_char(unsigned char * buffer, unsigned char value) {
    buffer[0] = value;
    return buffer + 1;
}

unsigned char * serialize_string(unsigned char * buffer, unsigned char * str, unsigned int length) {
    int i = 0;
    do {
        buffer = serialize_char(buffer, *str);
        str++;
    } while (++i != length);
    return buffer;
}

/**
 * Serializes the corresponding fields for the type of the msg
 */
unsigned char * serialize_request(unsigned char * buffer, request * request) {
    /** version serialization */
    buffer = serialize_char(buffer, request->version);

    /** command serialization */
    buffer = serialize_char(buffer, request->cmd);

    /** data size serialization */
    buffer = serialize_int(buffer, request->length);

    /** data serialization if present */
    if (request->length > 0) {
        buffer = serialize_string(buffer, request->data, request->length);
    }

    return buffer;
}

