/**
 * response.c -- parser del response de POP3
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "include/response.h"

#define RESP_SIZE (ERR + 1)

// ignoramos la descripcion de la response
struct pop3_response {
    const enum pop3_response_status status;
    const char 						*name;
};

static struct pop3_response responses[RESP_SIZE] = {
        {
                .status = OK,
                .name 	= "+OK",
        },
        {
                .status = ERR,
                .name	= "-ERR",
        },
};

#define N(x) (sizeof(x)/sizeof((x)[0]))

enum pop3_response_status
parse_response(const char *response) {

    for (int i = 0; i < N(responses); i++) {
        if (strcmp(response, responses[i].name) == 0) {
            return responses[i].status;
        }
    }

    return -1;
}