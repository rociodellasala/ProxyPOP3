#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "include/client_request.h"
#include "include/utils.h"

#define N(x) (sizeof(x)/sizeof((x)[0]))
#define CMD_SIZE	(capa + 1)

const struct pop3_request_cmd commands[CMD_SIZE] = {
        {
                .id 	= user,
                .name 	= "user",
        },{
                .id 	= pass,
                .name 	= "pass",
        },{
                .id 	= retr,
                .name 	= "retr",
        },{
                .id 	= list,
                .name 	= "list",
        },{
                .id 	= stat,
                .name 	= "stat",
        },{
                .id 	= dele,
                .name 	= "dele",
        },{
                .id 	= noop,
                .name 	= "noop",
        },{
                .id 	= top,
                .name 	= "top",
        },{
                .id 	= rset,
                .name 	= "rset",
        },{
                .id 	= quit,
                .name 	= "quit",
        },{
                .id 	= capa,
                .name 	= "capa",
        },
};

struct pop3_request_cmd invalid_cmd = {
        .id     = error,
        .name   = NULL,
};

const struct pop3_request_cmd * get_cmd(const char * cmd) {
    unsigned int i;
    char * aux = cmd;
    struct pop3_request_cmd * i_cmd;

    for (i = 0; i < CMD_SIZE; i++) {
        if (compare_strings(cmd, commands[i].name)) {
            return &commands[i];
        }
    }

    i_cmd = &invalid_cmd;
    i_cmd->name = aux;

    return i_cmd;
}

struct pop3_request * new_request(const struct pop3_request_cmd * cmd, char * args) {
    struct pop3_request * request = malloc(sizeof(*request));

    if (request == NULL) {
        return NULL;
    } else {
        request->cmd = cmd;
        request->args = args;

        return request;
    }
}



