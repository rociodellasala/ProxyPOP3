#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "include/client_request.h"
#include "include/utils.h"

#define CMD_QUANTITY   (capa + 1)

const struct pop3_request_cmd commands[CMD_QUANTITY] = {
        {
                .id 	    = user,
                .max_params = 1,
                .name 	    = "user",
        },{
                .id 	    = pass,
                .max_params = 1,
                .name 	    = "pass",
        },{
                .id 	    = retr,
                .max_params = 1,
                .name 	    = "retr",
        },{
                .id 	    = list,
                .max_params = 1,
                .name 	    = "list",
        },{
                .id 	    = stat,
                .max_params = 0,
                .name 	    = "stat",
        },{
                .id 	    = dele,
                .max_params = 1,
                .name 	    = "dele",
        },{
                .id 	    = noop,
                .max_params = 0,
                .name 	    = "noop",
        },{
                .id 	    = top,
                .max_params = 2,
                .name 	    = "top",
        },{
                .id 	    = rset,
                .max_params = 1,
                .name 	    = "rset",
        },{
                .id 	    = uidl,
                .max_params = 1,
                .name 	    = "uidl",
        },{
                .id 	    = quit,
                .max_params = 0,
                .name 	    = "quit",
        },{
                .id 	    = capa,
                .max_params = 1,
                .name 	    = "capa",
        },
};

struct pop3_request_cmd invalid_cmd = {
        .id         = error,
        .max_params = 0,
        .name       = NULL,
};

const struct pop3_request_cmd * get_cmd(const char * cmd) {
    unsigned int                i;
    struct pop3_request_cmd *   i_cmd;
    const char *                aux = cmd;

    for (i = 0; i < CMD_QUANTITY; i++) {
        if (compare_strings(cmd, commands[i].name)) {
            return &commands[i];
        }
    }

    i_cmd       = &invalid_cmd;
    i_cmd->name = aux;

    return i_cmd;
}

int get_max_parameter(const char * cmd) {
    const struct pop3_request_cmd * current_cmd = get_cmd(cmd);
    return current_cmd->max_params;
}

struct pop3_request * new_request(const struct pop3_request_cmd * cmd, char * args) {
    struct pop3_request * request = malloc(sizeof(*request));

    if (request == NULL) {
        return NULL;
    } else {
        request->cmd    = (struct pop3_request_cmd *) cmd;
        request->args   = args;
        return request;
    }
}



