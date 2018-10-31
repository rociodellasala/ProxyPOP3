/**
 * response.c -- parser del response de POP3
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "include/response.h"

#define RESP_SIZE (response_status_err + 1)

static const struct pop3_response responses[RESP_SIZE] = {
        {
                .status = response_status_ok,
                .name 	= "+OK",
        },
        {
                .status = response_status_err,
                .name	= "-ERR",
        },
};

static const struct pop3_response invalid_response = {
        .status = response_status_invalid,
        .name   = NULL,
};

#define N(x) (sizeof(x)/sizeof((x)[0]))

const struct pop3_response *
get_response(const char *response) {
    for (unsigned i = 0; i < N(responses); i++) {
        if (strcmp(response, responses[i].name) == 0) {
            return &responses[i];
        }
    }

    return &invalid_response;
}