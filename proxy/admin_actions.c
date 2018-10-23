#include <string.h>
#include "include/request_admin.h"
#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <netinet/sctp.h>
#include "include/serializer.h"
#include "include/response_admin.h"
#include "include/admin_actions.h"

int check_password(request * request, int * status) {
    unsigned char * data = request->data;
    unsigned char * password = "1234";
    int password_length = strlen(password);
    int data_length = strlen(data);
    
    if(password_length != data_length){
        *status = 0;
    } else {
        *status = 1;
    }
}

void forbid_mime(request * request, int * status, char * media_types){
    int ret; //check_mime_not_in(request->data, media_types);
    if(ret == -1) {
        //add_mime();
    } else {
        *status = 0;
    }
}

void allow_mime(request * request, int * status, char * media_types){
    int ret; //check_mime_not_in(request->data, media_types);
    printf("ret: %d\n", ret);
    if(ret == -1) {
        //remove_mime();
    } else {
        *status = 0;
    }
}

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

    sent_bytes = sctp_sendmsg(socket, buffer_response, pointer-buffer_response, NULL, 0, 0, 0, 0, 0, 0);

    if (sent_bytes <= 0) {
        printf("%s\n", strerror(errno));
    }

    return sent_bytes;
}