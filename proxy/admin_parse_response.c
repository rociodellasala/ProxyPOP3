#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/sctp.h>
#include <errno.h>
#include "include/admin.h"
#include "include/serializer.h"
#include "include/utils.h"

typedef int file_descriptor;

void ehlo(struct admin * admin){
    ssize_t send_bytes;
    char * w_message = "Welcome to POP3 Proxy Management Server\n";
    send_bytes = send(admin->fd, w_message, strlen(w_message), 0);

    if(send_bytes <= 0){
        admin->resp_status = COULD_NOT_SEND_WELCOME;
    } else {
        admin->a_status = ST_CONNECTED;
    }
}


ssize_t send_response(struct admin * admin, response_admin * response) {
    ssize_t sent_bytes;
    unsigned char buffer_response[MAX_ADMIN_BUFFER]; 
    unsigned char * pointer = serialize_response(buffer_response, response);

    sent_bytes = sctp_sendmsg(admin->fd, buffer_response, pointer-buffer_response, NULL, 0, 0, 0, 0, 0, 0);

    if (sent_bytes <= 0) {
        admin->resp_status = COULD_NOT_SEND_RESPONSE;
    }

    return sent_bytes;
}


void send_response_with_data(struct admin * admin, unsigned char status) {
    response_admin * response   = malloc(sizeof(*response));
    response->version           = VERSION;
    response->status            = status;
    response->length            = (unsigned int) strlen((const char *) admin->resp_data);
    response->data              = malloc(response->length * sizeof(unsigned char *));
    
    strncpy((char *) response->data, (const char *) admin->resp_data, response->length);
    
    send_response(admin, response);
    
    free(response->data);
    free(response);
}

void send_response_without_data(struct admin * admin, unsigned char status) {
    response_admin * response   = malloc(sizeof(*response));
    response->version           = VERSION;
    response->status            = status;
    response->length            = 0;
    response->data              = 0;
    
    send_response(admin, response);
    
    free(response);
}


int parse_admin_response(struct admin * admin) {
    unsigned char status;
    if (admin->a_status == ST_EHLO){
        ehlo(admin);
    } else {
        if(admin->resp_length != 0){
            if(admin->resp_status != RESP_PARSE_OK || admin->req_status != REQ_PARSE_OK) {
                send_response_with_data(admin, 0);
            } else {
                send_response_with_data(admin, 1);
            }
        } else {
            if(admin->resp_status != RESP_PARSE_OK){
                status = 0;
            } else {
                if(admin->req_status != REQ_PARSE_OK){
                    status = 0;
                } else {
                    status = 1;
                }
            }
            send_response_without_data(admin, status);
        }
    }

    if(admin->resp_status == COULD_NOT_SEND_RESPONSE || admin->resp_status == COULD_NOT_SEND_WELCOME){
        return -1;
    }

    return 0;
}

