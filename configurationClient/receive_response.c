#include <errno.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <string.h>
#include "include/deserializer.h"
#include "include/response.h"
#include "include/utils.h"

ssize_t receive_response(unsigned char * status, response * response, const file_descriptor socket) {
    ssize_t read_quan;
    unsigned char buffer[100];
    
    read_quan = sctp_recvmsg(socket, buffer, 100, NULL, 0, 0, 0);

    if (read_quan < 0) {
        printf("%s\n", strerror(errno));
    }

    deserialize_response(buffer, response);

    *status = response->status;
   
    return read_quan;
}