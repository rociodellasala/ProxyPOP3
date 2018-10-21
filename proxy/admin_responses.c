#include <string.h>
#include <malloc.h>
#include <errno.h>
#include "include/administrator.h"
#include "include/serializer.h"
#include "include/response.h"

typedef int file_descriptor;

void send_response_with_data(char * parameter, file_descriptor socket, int status) {
    response * response = malloc(sizeof(response));
    response->version = VERSION;
    response->status = status;
    response->length = (unsigned int) strlen(parameter);
    response->data = (unsigned char *) malloc(response->length * sizeof(char *));
    strncpy((char *)response->data, parameter, response->length);
    
    send_response(socket, response);
}

void send_response_without_data(file_descriptor socket, int status) {
    response * response = malloc(sizeof(response));
    response->version = VERSION;
    response->status = status;
    response->length = 0;
    response->data = 0;
    send_response(socket, response);
}


ssize_t send_response(file_descriptor socket, response * response) {
    ssize_t sent_bytes;
    unsigned char buffer_response[40];
    unsigned char * pointer = serialize_response(buffer_response, response);

    sent_bytes = send(socket, buffer_response, pointer-buffer_response, 0);

    if (sent_bytes <= 0) {
        printf("%s\n", strerror(errno));
    }

    return sent_bytes;
}