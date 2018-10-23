/**
 * request.c -- parser del request de POP3
 */
#include <string.h> // memset
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/request_parser.h"

static enum request_state cmd(const uint8_t c, struct request_parser * p) {
    enum request_state ret = request_cmd;
    struct pop3_request *r = p->request;

    if (c == ' ' || c == '\n') {
        r->cmd = parse_cmd(p->cmd_buffer);
        if (c == ' ') {
            ret = request_param;
        } else {
            ret = request_done;
        }
    } else if (p->i >= CMD_SIZE) {
        ret = request_error_cmd_too_long;
    } else {
        p->cmd_buffer[p->i++] = c;
    }

    return ret;
}

static enum request_state param(const uint8_t c, struct request_parser * p) {
    enum request_state ret = request_param;
    struct pop3_request *r = p->request;

    if (c == '\n') {
        // la alocacion se podria hacer antes de crear la nueva request, pero es lo mismo
        r->args = malloc(strlen(p->param_buffer));
        strcpy(r->args, p->param_buffer);
        ret = request_done;
    } else {
        p->param_buffer[p->j++] = c;
        if (p->j >= PARAM_SIZE) {
            ret = request_error_param_too_long;
        }
    }

    return ret;
}

extern void request_parser_init (struct request_parser * p) {
    memset(p->request, 0, sizeof(*(p->request)));
    memset(p->cmd_buffer, 0, CMD_SIZE);
    memset(p->param_buffer, 0, PARAM_SIZE);
    p->state = request_cmd;
    p->i = p->j = 0;
}

extern enum request_state request_parser_feed (struct request_parser* p, const uint8_t c) {
    enum request_state next;

    switch (p->state) {
        case request_cmd:
            next = cmd(c, p);
            break;
        case request_param:
            next = param(c, p);
            break;
        case request_done:
        case request_error:
        case request_error_cmd_too_long:
        case request_error_param_too_long:
            next = p->state;
            break;
        default:
            next = request_error;
            break;
    }

    return p->state = next;
}

extern bool request_is_done(const enum request_state st, bool *errored) {
    if(st >= request_error && errored != 0) {
        *errored = true;
    }
    return st >= request_done;
}

extern enum request_state request_consume(buffer *b, struct request_parser *p, bool *errored) {
    enum request_state st = p->state;

    while (buffer_can_read(b)) {
        const uint8_t c = buffer_read(b);
        st = request_parser_feed(p, c);
        if (request_is_done(st, errored)) {
            break;
        }
    }
    return st;
}

extern void request_close(struct request_parser *p) {
    // nada que hacer
}

extern int request_marshall(struct pop3_request *r, buffer *b) {
    size_t  n;
    uint8_t *buff;

    const char * cmd = r->cmd->name;
    char * args = r->args;

    size_t i = strlen(cmd);
    size_t j = args == NULL ? 0 : strlen(args);
    //size_t count = i + j + (j == 0 ? 2 : 3);
    size_t count = i + j + (j == 0 ? 1 : 2);

    buff = buffer_write_ptr(b, &n);

    if(n < count) {
        return -1;
    }

    memcpy(buff, cmd, i);
    buffer_write_adv(b, i);

    buffer_write(b, ' ');

    if (args != NULL) {
        buff = buffer_write_ptr(b, &n);
        memcpy(buff, args, j);
        buffer_write_adv(b, j);
    }

    //buffer_write(b, '\r');
    buffer_write(b, '\n');

    return (int)count;
}
