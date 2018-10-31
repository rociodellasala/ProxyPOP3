/**
 * request.c -- parser del request de POP3
 */
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/request_parser.h"

#define SPACE   ' '
#define CR      '\r'

static enum request_state cmd(const uint8_t c, struct request_parser * parser) {
    enum request_state next         = request_cmd;
    struct pop3_request * request   = parser->request;

    if (c == ' ' || c == '\r' || c == '\n') {
        request->cmd = (struct pop3_request_cmd *) get_cmd(parser->cmd_buffer);
        if (request->cmd->id == error) {
            next = request_error_inexistent_cmd;
        } else {
            switch (c) {
                case SPACE:
                    next = request_param;
                    parser->cmd_buffer[parser->i++] = '\0';
                    break;
                case CR:
                    next = request_newline;
                    break;
                default:
                    next = request_done;
                    break;
            }
        }
    } else if (parser->i >= CMD_SIZE) {
        next = request_error_cmd_too_long;
    } else {
        parser->cmd_buffer[parser->i++] = c;
    }

    return next;
}

static enum request_state param(const uint8_t c, struct request_parser * parser) {
    enum request_state ret          = request_param;
    struct pop3_request * request   = parser->request;

    if (c == '\r' || c == '\n') {
        char * aux = parser->param_buffer;
        int count = 0;
        while (*aux  != 0) {
            if (*aux == ' ' || *aux == '\t')
                count++;
            aux++;
        }

        if (count != parser->j) {
            request->args = malloc(strlen(parser->param_buffer) + 1);
            strcpy(request->args, parser->param_buffer);
        }

        if (c == '\r') {
            ret = request_newline;
        } else {
            ret = request_done;
        }
    } else {
        parser->param_buffer[parser->j++] = c;
        if (parser->j >= PARAM_SIZE) {
            ret = request_error_param_too_long;
        }
    }

    return ret;
}

extern void request_parser_init(struct request_parser * parser) {
    memset(parser->request, 0, sizeof(*(parser->request)));
    memset(parser->cmd_buffer, 0, CMD_SIZE);
    memset(parser->param_buffer, 0, PARAM_SIZE);
    parser->state = request_cmd;
    parser->i = parser->j = 0;
}

extern enum request_state request_parser_feed(struct request_parser * p, const uint8_t c) {
    enum request_state next;

    switch(p->state) {
        case request_cmd:
            next = cmd(c, p);
            break;
        case request_param:
            next = param(c, p);
            break;
        case request_newline:
            if (c != '\n') {
                next = request_error_inexistent_cmd;
            } else {
                next = request_done;
            }
            break;
        case request_done:
        case request_error_inexistent_cmd:
        case request_error_cmd_too_long:
        case request_error_param_too_long:
            next = p->state;
            break;
        default:
            next = request_error_inexistent_cmd;
            break;
    }

    return p->state = next;
}

bool request_is_done(const enum request_state st, bool *errored) {
    if(st >= request_error_inexistent_cmd && errored != 0) {
        *errored = true;
    }
    return st >= request_done;
}

void clean_buffer(buffer * buffer, uint8_t c, enum request_state * st){
    while(buffer_can_read(buffer) && c != '\n') {
        c = buffer_read(buffer);
    }

    if (c != '\n') {
        *st = request_cmd;
    }
}

enum request_state request_consume(buffer * buffer, struct request_parser * parser, bool *errored) {
    enum request_state st = parser->state;
    uint8_t c = 0;

    while(buffer_can_read(buffer)) {
        c = buffer_read(buffer);
        st = request_parser_feed(parser, c);
        if(request_is_done(st, errored)) {
            break;
        }
    }

    if (st >= request_error_inexistent_cmd && c != '\n') {
       clean_buffer(buffer, c, &st);
    }

    return st;
}

extern int
request_marshall(struct pop3_request *r, buffer *b) {
    size_t  n;
    uint8_t *buff;

    const char * cmd = r->cmd->name;
    char * args = r->args;

    size_t i = strlen(cmd);
    size_t j = args == NULL ? 0 : strlen(args);
    size_t count = i + j + (j == 0 ? 2 : 3);

    buff = buffer_write_ptr(b, &n);

    if(n < count) {
        return -1;
    }

    memcpy(buff, cmd, i);
    buffer_write_adv(b, i);


    if (args != NULL) {
        buffer_write(b, ' ');
        buff = buffer_write_ptr(b, &n);
        memcpy(buff, args, j);
        buffer_write_adv(b, j);
    }

    buffer_write(b, '\r');
    buffer_write(b, '\n');

    return (int)count;
}
