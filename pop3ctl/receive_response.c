#include <errno.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <string.h>
#include "include/deserializer.h"
#include "include/response.h"
#include "include/utils.h"
#include "include/administrator.h"

ssize_t receive_response(enum response_status * status, struct response * response) {
    ssize_t read_quan;
    unsigned char buffer[MAX_BUFFER];
    
    read_quan = sctp_recvmsg(socket_fd, buffer, MAX_BUFFER, NULL, 0, 0, 0);

    if (read_quan < 0) {
        printf("%s\n", strerror(errno));
    }

    deserialize_response(buffer, response);

    *status = (enum response_status) response->status;
   
    return read_quan;
}