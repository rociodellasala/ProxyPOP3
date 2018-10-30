#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

#include "include/request.h"

/** Ejecuta una transformacion externa luego de un retrieve */
void retr_fn(struct pop3_request *r) {

    printf("Ejecutar transformacion externa\n");
}

/** Placeholder para comandos que no necesitan ejecutar nada */
void common_fn(struct pop3_request *r) {
    //printf("Nada por hacer\n");
}

const struct pop3_request_cmd commands[CMD_SIZE] = {
        {
                .id 	= user,
                .name 	= "user",
                .fn     = common_fn,
        },{
                .id 	= pass,
                .name 	= "pass",
                .fn     = common_fn,
        },{
                .id 	= apop,
                .name 	= "apop",
                .fn     = common_fn,
        },{
                .id 	= stat,
                .name 	= "stat",
                .fn     = common_fn,
        },{
                .id 	= list,
                .name 	= "list",
                .fn     = common_fn,
        },{
                .id 	= retr,
                .name 	= "retr",
                .fn     = retr_fn,
        },{
                .id 	= dele,
                .name 	= "dele",
                .fn     = common_fn,
        },{
                .id 	= noop,
                .name 	= "noop",
                .fn     = common_fn,
        },{
                .id 	= rset,
                .name 	= "rset",
                .fn     = common_fn,
        },{
                .id 	= top,
                .name 	= "top",
                .fn     = common_fn,
        },{
                .id 	= uidl,
                .name 	= "uidl",
                .fn     = common_fn,
        },{
                .id 	= quit,
                .name 	= "quit",
                .fn     = common_fn,
        }
};

const struct pop3_request_cmd invalid_cmd = {
        .id     = error,
        .name   = NULL,
        .fn     = NULL,
};


/**
 * Comparacion case insensitive de dos strings
 */
static bool strcmpi(const char * str1, const char * str2) {
    int c1, c2;
    while (*str1 && *str2) {
        c1 = tolower(*str1++);
        c2 = tolower(*str2++);
        if (c1 != c2) {
            return false;
        }
    }

    return *str1 == 0 && *str2 == 0 ? true : false;
}

#define N(x) (sizeof(x)/sizeof((x)[0]))

const struct pop3_request_cmd * parse_cmd(const char * cmd) {
    for (int i = 0; i < N(commands); i++) {
        if (strcmpi(cmd, commands[i].name)) {
            return &commands[i];
        }
    }

    return &invalid_cmd;
}
