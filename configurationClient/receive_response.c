
#include <monetary.h>
#include <netinet/sctp.h>
#include <errno.h>
#include "include/response.h"
#include "include/administrator.h"
#include "include/deserializer.h"

ssize_t receive_response(file_descriptor socket, response * response, int * status) {
    ssize_t read_quan;
    unsigned char buffer[40];
    
    read_quan = sctp_recvmsg(socket, buffer, 40, NULL,0,0,0);

    if (read_quan < 0) {
        printf("%s\n", strerror(errno));
    }

    deserialize_response(buffer, response);
    *status = response->status;
   
    return read_quan;
}