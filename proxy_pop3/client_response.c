/**
 * response.c -- parser del response de POP3
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "include/client_response.h"
#include "include/utils.h"

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

const struct pop3_response * get_response(const char * response) {
    unsigned i;

    for (i = 0; i < N(responses); i++) {
        if (compare_strings(response, responses[i].name) == true) {
            return &responses[i];
        }
    }

    return &invalid_response;
}