
#include <string.h>
#include "include/request.h"
#include <stdio.h>
#include "include/media_types_utils.h"

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
    int ret = check_mime_not_in(request->data, media_types);
    if(ret == -1) {
        //add_mime();
    } else {
        *status = 0;
    }
}

void allow_mime(request * request, int * status, char * media_types){
    int ret = check_mime_not_in(request->data, media_types);
    printf("ret: %d\n", ret);
    if(ret == -1) {
        //remove_mime();
    } else {
        *status = 0;
    }
}