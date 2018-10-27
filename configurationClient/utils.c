#include <stdio.h>
#include <string.h>
#include "include/response.h"

void print_msg(response_status status, response response) {
    int i; 
  
    if (response.length > 0 && response.data != NULL) {
        response.data;

        if (status == OK) {
            printf("Answer from proxy: +OK: %s\n", response.data);
        } else {
            printf("Answer from proxy: -ERR: %s\n", response.data);
        }
    } else {
        if (status == OK) {
            printf("Answer from proxy: +OK\n");
        } else {
            printf("Answer from proxy: -ERR\n");
        }
    }
}

void show_menu_authentication() {
    printf(
            "\nINPUT FORMAT:     [COMMAND_NUMBER]  [parameter]\n"
            "  - AUTHENTICATE:           1            pass\n"
            "  - QUIT:                   9\n");
}

void show_menu_transaction() {
    printf(
            "\nINPUT FORMAT:                  [COMMAND_NUMBER]        [parameter]\n"
            "  - SET TRANSFORMATION PROGRAM:          2          transformationprogram\n"
            "  - GET TRANSFORMATION PROGRAM:          3 \n"
            "  - SWITCH TRANSFORMATION PROGRAM:       4 \n"
            "  - GET METRIC:                          5                 metric\n"
            "  - GET MIME:                            6 \n"
            "  - ALLOW MIME:                          7                  mime\n"
            "  - FORBID MIME:                         8                  mime\n"
            "  - QUIT:                                9\n");
}