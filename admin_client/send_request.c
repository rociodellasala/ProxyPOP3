#include <errno.h>
#include <malloc.h>
#include <netinet/sctp.h>
#include <string.h>

#include "include/client_request.h"
#include "include/send_request.h"
#include "include/serializer.h"
#include "include/admin.h"

ssize_t send_request(struct request * request) {
    ssize_t         sent_bytes;
    unsigned char   buffer[MAX_BUFFER];
    unsigned char * pointer = serialize_request(buffer, request);

    sent_bytes = sctp_sendmsg(socket_fd, buffer, pointer-buffer, NULL, 0, 0, 0, 0, 0, 0);

    if (sent_bytes <= 0) {
        printf("%s\n", strerror(errno));
    }

    return sent_bytes;
}

void send_request_one_param(const char * parameter, b_cmd cmd) {
    struct request * request = malloc(sizeof(*request));

    request->version    = VERSION;
    request->cmd        = cmd;
    request->length     = (unsigned int) strlen(parameter) + 1;
    request->data       = malloc(request->length * sizeof(char *));
    strncpy((char *) request->data, parameter, request->length);

    send_request(request);
    
    free(request->data);
    free(request);
}

void send_request_without_param(b_cmd cmd) {
    struct request * request = malloc(sizeof(*request));

    request->version    = VERSION;
    request->cmd        = cmd;
    request->length     = 0;
    request->data       = 0;

    send_request(request);
    
    free(request);
}

