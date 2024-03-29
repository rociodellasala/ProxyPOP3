#include <stdio.h>
#include <string.h>

#include "include/response.h"

void print_msg(const enum response_status status, const struct response response) {
    if (response.length > 0 && response.data != NULL) {
        if (status == OK) {
            printf("Answer from proxy_pop3: +OK: %s\n", response.data);
        } else {
            printf("Answer from proxy_pop3: -ERR: %s\n", response.data);
        }
    } else {
        if (status == OK) {
            printf("Answer from proxy_pop3: +OK\n");
        } else {
            printf("Answer from proxy_pop3: -ERR\n");
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
            "\nINPUT FORMAT:                   [COMMAND_NUMBER]        [parameter]\n"
            "  - SET TRANSFORMATION PROGRAM:          2          transformationprogram\n"
            "  - GET TRANSFORMATION PROGRAM:          3 \n"
            "  - SWITCH TRANSFORMATION PROGRAM:       4 \n"
            "  - GET METRIC:                          5                 metric\n"
            "                                                           where metric is:     CONCURRENT CONNECTIONS        0\n"
            "                                                                                MAX CONCURRENT CONNECTIONS    1\n"
            "                                                                                HISTORICAL ACCESSES           2\n"
            "                                                                                TRANSFERED BYTES              3\n"
            "                                                                                CURRENT ADMINS CONNECTED      4\n"
            "                                                                                MAX ADMINS CONNECTED          5\n"
            "  - GET MIME:                            6 \n"
            "  - ALLOW MIME:                          7                  mime\n"
            "  - FORBID MIME:                         8                  mime\n"
            "  - QUIT:                                9\n");
}