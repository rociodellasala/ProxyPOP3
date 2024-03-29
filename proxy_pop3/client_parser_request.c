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
        // si es un espacio o nueva linea entonces termino el comando, lo proceso
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
        // si me pase del tamaño entonces el comando es invalido
        request->cmd = (struct pop3_request_cmd *) get_cmd(parser->cmd_buffer);
        next         = request_error_cmd_too_long;
    } else {
        // sino guardo el el buffer lo ingresado
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

static enum request_state parameter(const uint8_t c, struct request_parser * parser, int max_param) {
    enum request_state      ret       = request_parameter;
    struct pop3_request *   request   = parser->request;

    // contemplo los casos en los cuales la request tenga espacios de mas
    if (c == SPACE) {
        return ret;
    }

    if (strcmp(parser->cmd_buffer, "TOP") == 0 && c != SPACE && c != CR && c != NEWLINE) {
        if(parser->params == 1){
            parser->param_buffer[parser->params][parser->j++] = c;
            parser->param_buffer[parser->params][parser->j++] = '\0';
        } else if (parser->params == 0) {
            parser->param_buffer[parser->params][parser->j++] = c;
            parser->params++;
            parser->j = 0;
        }

        return ret;
    }

    if (c == CR) {
        return ret;
    }

    if (c == NEWLINE) {
        if (parser->j == 0 && parser->params == 0 && parser->request->cmd->max_params == 0) {
            return request_done;
        } if (parser->j == 0 && parser->params == 0 && parser->request->cmd->max_params == 1) {
            return request_done;
        } else {
            char * aux = parser->param_buffer[parser->params];
            parser->param_buffer[parser->params][parser->j++] = '\0';
            parser->params++;

            int count = 0;

            while (*aux != 0) {
                if (*aux == ' ' || *aux == '\t') {
                    count++;
                }

                aux++;
            }

            if (parser->params != 0) {
                assemble_parameters(count, parser, request);
            }


            ret = request_done;

        }
    } else {
        parser->param_buffer[parser->params][parser->j++] = c;

        if (parser->j >= MAX_PARAM_SIZE - 1) {
            ret = request_error_param_too_long;
        }
    }

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

extern enum request_state request_parser_feed(struct request_parser * parser, const uint8_t c, int * max_param) {
    enum request_state next;

    // estados para saber que parte del request estoy parseando
    switch(parser->state) {
        case request_cmd:
            next = cmd(c, parser);
            if(next == request_cmd){
                *max_param = get_max_parameter(parser->cmd_buffer);
            }
            break;
        case request_parameter:
            next = parameter(c, parser, *max_param);
            break;
        case request_newline:
            next = newline(c);
            break;
        case request_done:
        case request_error_inexistent_cmd:
        case request_error_cmd_too_long:
        case request_error_param_too_long:
        case request_error_too_many_params:
            next = parser->state;
            break;
        default:
            next = request_error_inexistent_cmd;
            break;
    }

    return parser->state = next;
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
    int max_param;

    while (buffer_can_read(buffer)) {
        c   = buffer_read(buffer);
        st  = request_parser_feed(parser, c, &max_param);
        // parseo char a char la request
        if (request_is_done(st, errored)) {
            break;
        }
    }

    if (st >= request_error_inexistent_cmd && c != '\n') {
       clean_buffer(buffer, c, &st);
    }

    return st;
}

