#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/sctp.h>
#include <errno.h>
#include "include/admin.h"
#include "include/serializer.h"

typedef int file_descriptor;

void ehlo(struct admin * admin){
    ssize_t send_bytes;
    char * w_message = "Welcome to POP3 Proxy Management Server\n";
    send_bytes = send(admin->fd, w_message, strlen(w_message), 0);

    if(send_bytes <= 0){
        admin->resp_status = COULD_NOT_SEND_RESPONSE;
    } else {
        admin->a_status = ST_AUTH;
    }
}


ssize_t send_response(file_descriptor socket, response_admin * response) {
    ssize_t sent_bytes;
<<<<<<< HEAD
    unsigned char buffer_response[40];
    unsigned char * pointer = serialize_response(buffer_response, response);
    puts("ACAAAAAAAAA");
=======
    unsigned char buffer_response[100];
    unsigned char * pointer = serialize_response(buffer_response, response);

>>>>>>> f63ebf25f7c99555cb02535cd0d41dc4fef5c2bc
    sent_bytes = sctp_sendmsg(socket, buffer_response, pointer-buffer_response, NULL, 0, 0, 0, 0, 0, 0);

    if (sent_bytes <= 0) {
        printf("%s\n", strerror(errno));
    }

    return sent_bytes;
}


<<<<<<< HEAD
void send_response_with_data(char * parameter, file_descriptor socket) {
    puts("AAAAAAAAAAAAA");
    response_admin * response   = malloc(sizeof(*response));
    response->version           = VERSION;
    response->status            = 1;
=======
void send_response_with_data(char * parameter, file_descriptor socket, unsigned char status) {
    response_admin * response   = malloc(sizeof(*response));
    response->version           = VERSION;
    response->status            = status;
>>>>>>> f63ebf25f7c99555cb02535cd0d41dc4fef5c2bc
    response->length            = (unsigned int) strlen(parameter);
    response->data              = malloc(response->length * sizeof(unsigned char *));
    
    strncpy(response->data, parameter, response->length);
    
    send_response(socket, response);
    
    free(response->data);
    free(response);
}

void send_response_without_data(file_descriptor socket, unsigned char status) {
<<<<<<< HEAD
    puts("ENNNNNNNNNNNNNNNNN");
=======
>>>>>>> f63ebf25f7c99555cb02535cd0d41dc4fef5c2bc
    response_admin * response   = malloc(sizeof(*response));
    response->version           = VERSION;
    response->status            = status;
    response->length            = 0;
    response->data              = 0;
    
    send_response(socket, response);
    
    free(response);
}

<<<<<<< HEAD
=======

>>>>>>> f63ebf25f7c99555cb02535cd0d41dc4fef5c2bc
void parse_admin_response(struct admin * admin) {
    int status;
    if (admin->a_status == ST_EHLO){
        ehlo(admin);
    } else {
        if(admin->resp_length != 0){
<<<<<<< HEAD
            send_response_with_data(admin->resp_data, admin->fd);
        } else {
            if(admin->req_status != RESP_PARSE_OK){
                status = 0;
            } else {
                status = 1;
=======
            if(admin->resp_status != RESP_PARSE_OK || admin->req_status != REQ_PARSE_OK) {
                send_response_with_data(admin->resp_data, admin->fd, 0);
            } else {
                send_response_with_data(admin->resp_data, admin->fd, 1);
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
>>>>>>> f63ebf25f7c99555cb02535cd0d41dc4fef5c2bc
            }
            send_response_without_data(admin->fd, status);
        }
    }
}

