
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <netinet/sctp.h>
#include "include/request.h"
#include "include/send_request.h"
#include "include/serializer.h"

#define VERSION 1

void send_request_one_param(enum req_cmd cmd, const char * parameter, file_descriptor socket) {
    request * request = malloc(sizeof(*request));
    request->version = VERSION;
    request->cmd = cmd;
    request->length = (unsigned int) strlen(parameter) + 1;
    request->data = malloc(request->length * sizeof(unsigned char *));
    strncpy((char *)request->data, parameter, request->length);

    send_request(socket, request);
    
    free(request->data);
    free(request);
}

void send_request_without_param(enum req_cmd cmd, file_descriptor socket) {
    request * request   = malloc(sizeof(*request));
    request->version    = VERSION;
    request->cmd        = cmd;
    request->length     = 0;
    request->data       = 0;

    send_request(socket, request);
    
    free(request);
}

ssize_t send_request(file_descriptor socket, request * request) {
    ssize_t sent_bytes;
    unsigned char buffer[100];
    unsigned char * pointer = serialize_request(buffer, request);

    sent_bytes = sctp_sendmsg(socket, buffer, pointer-buffer, NULL, 0, 0, 0, 0, 0, 0);

    if (sent_bytes <= 0) {
        printf("%s\n", strerror(errno));
    }

    return sent_bytes;
}
