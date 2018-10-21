#include "include/serializer.h"
#include <stdlib.h>
#include <printf.h>
#include <stdio.h>
#include <string.h>
#include "include/request.h"
#include "include/response.h"

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
unsigned char * serialize_response(unsigned char * buffer, response * response) {

    /** version serialization */
    buffer = serialize_char(buffer, response->version);
    /** command serialization */
    buffer = serialize_char(buffer, response->status);
    /** data size serialization */
    buffer = serialize_int(buffer, response->length);
    /** data serialization if present */
    if (response->length > 0) {
        buffer = serialize_string(buffer, response->data, response->length);
    }

    return buffer;
}

