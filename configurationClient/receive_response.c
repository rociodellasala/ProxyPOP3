#include <errno.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <string.h>
#include "include/deserializer.h"
#include "include/response.h"
#include "include/utils.h"
#include "include/administrator.h"

ssize_t receive_response(response_status * status, response * response) {
    ssize_t read_quan;
    unsigned char buffer[100];
    
    read_quan = sctp_recvmsg(socket_fd, buffer, 100, NULL, 0, 0, 0);

    if (read_quan < 0) {
        printf("%s\n", strerror(errno));
    }

    deserialize_response(buffer, response);

    *status = (response_status) response->status;
   
    return read_quan;
}