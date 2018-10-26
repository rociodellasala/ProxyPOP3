#include <stdio.h>
#include <string.h>
#include "include/response.h"

void print_msg(int status, response response) {
    char msg[response.length + 1];
    int i;
    for( i = 0; i < response.length; i++){
        msg[i] = response.data[i];
    }
    
    msg[i] = '\0';
    
    if(response.length > 0 && response.data != NULL) {
        response.data;

        if (status) {
            printf("Answer from proxy: +OK: %s\n", msg);
        } else {
            printf("Answer from proxy: -ERR: %s\n", msg);
        }
    } else {
        if (status) {
            printf("Answer from proxy: +OK\n");
        } else {
            printf("Answer from proxy: -ERR\n");
        }
    }
    
}

void show_menu() {
    printf(
            "\n-------------------------- MENU --------------------------\n"
            "FORMAT: [COMMAND_NUMBER] [parameter]\n"
            " - AUTENTHICATION:                1 pass\n"
            " - SET TRANSFORMATION PROGRAM:    2 transformationprogram\n"
            " - GET TRANSFORMATION PROGRAM:    3 \n"
            " - SWITCH TRANSFORMATION PROGRAM: 4 \n"
            " - GET METRIC:                    5 metric\n"
            " - GET MIME:                      6 \n"
            " - ALLOW MIME:                    7 mime\n"
            " - FORBID MIME:                   8 mime\n"
            " - QUIT:                          9\n"
            "USAGE EXAMPLE: 2 ./cat\n");
}

