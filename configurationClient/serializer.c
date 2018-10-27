#include "include/request.h"

unsigned char * serialize_int(unsigned char * buffer, unsigned int value) {
    buffer[0] = (unsigned char)(value >> 24 & 0xFF);
    buffer[1] = (unsigned char)(value >> 16 & 0xFF);
    buffer[2] = (unsigned char)(value >> 8 & 0xFF);
    buffer[3] = (unsigned char)(value & 0xFF);
    
    return buffer + 4;
}

unsigned char * serialize_char(unsigned char * buffer,const unsigned char value) {
    buffer[0] = value;
    
    return buffer + 1;
}

unsigned char * serialize_string(unsigned char * buffer, unsigned char * str, const unsigned int length) {
    unsigned int i = 0;
    
    do {
        buffer = serialize_char(buffer, *str);
        str++;
    } while (++i != length);
    
    return buffer;
}


unsigned char * serialize_request(unsigned char * buffer, request * request) {
    /* serializamos la version del proxy */
    buffer = serialize_char(buffer, request->version);

    /* serializamos el comando a enviar */
    buffer = serialize_char(buffer, request->cmd);

    /* serializamos la longitud de los datos (puede ser 0) */
    buffer = serialize_int(buffer, request->length);

    /* serializamos la data a enviar (si la hay) */
    if (request->length > 0) {
        buffer = serialize_string(buffer, request->data, request->length);
    }

    return buffer;
}

