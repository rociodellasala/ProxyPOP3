/**
 * request.c -- parser del request de POP3
 */
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/client_parser_request.h"

#define SPACE   ' '
#define CR      '\r'
#define NEWLINE '\n'

static enum request_state cmd(const uint8_t c, struct request_parser * parser) {
    enum request_state next         = request_cmd;
    struct pop3_request * request   = parser->request;

    if (c == SPACE || c == CR || c == NEWLINE) {
        request->cmd = (struct pop3_request_cmd *) get_cmd(parser->cmd_buffer);
        if (request->cmd->id == error) {
            next = request_error_inexistent_cmd;
        } else {
            switch (c) {
                case SPACE:
                    parser->cmd_buffer[parser->i++] = '\0';
                    next = request_parameter;
                    break;
                case CR:
                    next = request_newline;
                    break;
                default:
                    next = request_done;
                    break;
            }
        }
    } else if (parser->i >= MAX_CMD_SIZE) {
        request->cmd = (struct pop3_request_cmd *) get_cmd(parser->cmd_buffer);
        next = request_error_cmd_too_long;
    } else {
        parser->cmd_buffer[parser->i++] = c;
    }

    return next;
}

void assemble_parameters(int count, struct request_parser * parser, struct pop3_request * request){
    int size = 0;
    int i;

    for (i = 0; i < parser->params; i++)  {
        size += strlen(parser->param_buffer[i]);
    }

    if (count != parser->j) {
        request->args = malloc((size + parser->params + 200) * sizeof(char *));
    }

    if (parser->params == 2) {
        strcpy(request->args, parser->param_buffer[0]);
        strcat(request->args, " ");
        strcat(request->args, parser->param_buffer[1]);
    } else if(parser->params == 1) {
        strcpy(request->args, parser->param_buffer[0]);
    }
}

static enum request_state parameter(const uint8_t c, struct request_parser * parser, int max_param,  bool * is_space) {
    enum request_state      ret       = request_parameter;
    struct pop3_request *   request   = parser->request;

    if (c == SPACE){
        *is_space = true;
        return ret;
    }

    if (*is_space && c != SPACE && c != CR && c != NEWLINE) {
        parser->param_buffer[parser->params][parser->j++] = '\0';
        parser->params++;
        parser->j = 0;
        if(parser->params == max_param){
            ret =  request_error_too_many_params;
            return ret;
        }
    }

    if (c == CR || c == NEWLINE) {
        char * aux  = parser->param_buffer[parser->params];
        parser->param_buffer[parser->params][parser->j++] = '\0';
        parser->params++;
        int count   = 0;

        while (*aux != 0) {
            if (*aux == ' ' || *aux == '\t') {
                count++;
            }

            aux++;
        }

        if (parser->params != 0) {
            assemble_parameters(count, parser, request);
        }

        if (c == CR) {
            ret = request_newline;
        } else {
            ret = request_done;
        }

    } else {
        parser->param_buffer[parser->params][parser->j++] = c;
        if (parser->j >= MAX_PARAM_SIZE - 1) {
            ret = request_error_param_too_long;
        }
    }

    *is_space = false;

    return ret;
}

static enum request_state newline(const uint8_t c) {
    if (c != '\n') {
        return request_error_inexistent_cmd;
    }

    return request_done;
}

extern void request_parser_reset(struct request_parser * parser) {
    memset(parser->request, 0, sizeof(*(parser->request)));
    memset(parser->cmd_buffer, 0, MAX_CMD_SIZE);
    memset(parser->param_buffer, 0, MAX_PARAM_SIZE);
    parser->state   = request_cmd;
    parser->i       = parser->j = 0;
    parser->params  = 0;
}

extern enum request_state request_parser_feed(struct request_parser * p, const uint8_t c, int * max_param, bool * is_space) {
    enum request_state next;

    switch(p->state) {
        case request_cmd:
            next = cmd(c, p);
            if(next == request_cmd){
                *max_param = get_max_parameter(p->cmd_buffer);
            }
            break;
        case request_parameter:
            next = parameter(c, p, *max_param, is_space);
            break;
        case request_newline:
            next = newline(c);
            break;
        case request_done:
        case request_error_inexistent_cmd:
        case request_error_cmd_too_long:
        case request_error_param_too_long:
        case request_error_too_many_params:
            next = p->state;
            break;
        default:
            next = request_error_inexistent_cmd;
            break;
    }

    return p->state = next;
}

bool request_is_done(const enum request_state st, bool * errored) {
    if(st >= request_error_inexistent_cmd && errored != 0) {
        *errored = true;
    }

    return st >= request_done;
}

void clean_buffer(buffer * buffer, uint8_t c, enum request_state * st) {
    while (buffer_can_read(buffer) && c != '\n') {
        c = buffer_read(buffer);
    }

    if (c != '\n') {
        *st = request_cmd;
    }

}

enum request_state request_consume(buffer * buffer, struct request_parser * parser, bool * errored) {
    enum request_state  st          = parser->state;
    uint8_t             c           = 0;
    bool                is_space    = false;
    int max_param;

    while (buffer_can_read(buffer)) {
        c   = buffer_read(buffer);
        st  = request_parser_feed(parser, c, &max_param, &is_space);
        if (request_is_done(st, errored)) {
            break;
        }
    }

    if (st >= request_error_inexistent_cmd && c != '\n') {
       clean_buffer(buffer, c, &st);
    }

    return st;
}

