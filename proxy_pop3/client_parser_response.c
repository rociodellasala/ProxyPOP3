/**
 * response.c -- parser del response de POP3
 */

#include <string.h> // memset
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "include/client_parser_response.h"
#include "include/pop3_multi.h"

enum response_state status(const uint8_t c, struct response_parser * parser) {
    enum response_state ret = response_status;

    if (c == ' ' || c == '\r') {
        parser->request->response = get_response(parser->status_buffer);
        parser->status_buffer[parser->i++] = '\0';

        if (parser->request->response->status == response_status_invalid) {
            ret = response_error;
        } else {
            if (c == ' ') {
                ret = response_description;
            } else {
                ret = response_newline;
            }
        }

    } else if (parser->i >= STATUS_SIZE) {
        ret = response_error;
    } else {
        parser->status_buffer[parser->i++] = c;
    }

    return ret;
}

//ignoramos la descripcion
enum response_state description(const uint8_t c) {
    if (c == '\r') {
        return response_newline;
    }

    return response_description;
}

enum response_state newline(const uint8_t c, struct response_parser * parser) {
    enum response_state ret = response_done;

    if (parser->request->response->status != response_status_err) {

        switch (parser->request->cmd->id) {
            case retr:
            case top:
                ret = response_get_mail;
                break;
            case list:
                if (parser->request->args == NULL) {
                    ret = response_list;
                }
                break;
            case capa:
                ret = response_capa;
                break;
            case uidl:
                if (parser->request->args != NULL) {
                   ret = response_done;
                } else {
                    ret = response_multiline;
                }
                break;
            default:
                break;
        }
    }

    if (c == '\n') {
        parser->first_line_consumed = true;
    }

    return c != '\n' ? response_error : ret;
}

enum response_state mail(const uint8_t c, struct response_parser * parser) {
    const struct parser_event * e   = parser_feed(parser->pop3_multi_parser, c);
    enum response_state         ret = response_get_mail;

    switch (e->type) {
        case POP3_MULTI_FIN:
            ret = response_done;
            break;
        default:
            break;
    }
    return ret;
}

enum response_state list_cmd(const uint8_t c, struct response_parser * parser) {
    const struct parser_event * e   = parser_feed(parser->pop3_multi_parser, c);
    enum response_state         ret = response_list;

    switch (e->type) {
        case POP3_MULTI_FIN:
            ret = response_done;
            break;
        default:
            break;
    }

    return ret;
}

enum response_state capability(const uint8_t c, struct response_parser * parser) {
    const struct parser_event * e   = parser_feed(parser->pop3_multi_parser, c);
    enum response_state         ret = response_capa;

    if (parser->j == parser->capa_size) {
        void * tmp = realloc(parser->capa_response, parser->capa_size + BLOCK_SIZE);

        if (tmp == NULL) {
            return response_error;
        }

        parser->capa_size += BLOCK_SIZE;
        parser->capa_response = tmp;
    }

    parser->capa_response[parser->j++] = toupper(c);

    switch (e->type) {
        case POP3_MULTI_FIN:
            if (parser->j == parser->capa_size) {
                void * tmp = realloc(parser->capa_response, parser->capa_size + 1);

                if (tmp == NULL) {
                    return response_error;
                }

                parser->capa_size++;
                parser->capa_response = tmp;
            }

            parser->capa_response[parser->j] = 0;
            ret = response_done;
            break;
        default:
            break;
    }

    return ret;
}

enum response_state multiline(const uint8_t c, struct response_parser * parser) {
    const struct parser_event * e = parser_feed(parser->pop3_multi_parser, c);
    enum response_state ret = response_multiline;

    switch (e->type) {
        case POP3_MULTI_FIN:
            ret = response_done;
            break;
        default:
            break;
    }

    return ret;
}

void response_parser_init(struct response_parser * parser) {
    memset(parser->status_buffer, 0, STATUS_SIZE);
    parser->state               = response_status;
    parser->first_line_consumed = false;
    parser->i                   = 0;

    if (parser->pop3_multi_parser == NULL) {
        parser->pop3_multi_parser = parser_init(parser_no_classes(), pop3_multi_parser());
    }

    parser_reset(parser->pop3_multi_parser);

    if (parser->capa_response != NULL) {
        parser->capa_response = NULL;
    }

    parser->j           = 0;
    parser->capa_size   = 0;
}

extern enum response_state response_parser_feed(struct response_parser * parser, const uint8_t c) {
    enum response_state next;

    switch (parser->state) {
        case response_status:
            next = status(c, parser);
            break;
        case response_description:
            next = description(c);
            break;
        case response_newline:
            next = newline(c, parser);
            break;
        case response_get_mail:
            next = mail(c, parser);
            break;
        case response_list:
            next = list_cmd(c, parser);
            break;
        case response_capa:
            next = capability(c, parser);
            break;
        case response_multiline:
            next = multiline(c, parser);
            break;
        case response_done:
        case response_error:
            next = parser->state;
            break;
        default:
            next = response_error;
            break;
    }

    return parser->state = next;
}

extern bool response_is_done(const enum response_state st, bool * errored) {
    if (st >= response_error && errored != 0) {
        *errored = true;
    }

    return st >= response_done;
}

extern enum response_state response_consume(buffer * read_buffer, buffer * write_buffer, struct response_parser * parser, bool * errored) {
    enum response_state st = parser->state;
    uint8_t             c  = 0;

    if (parser->state == response_done) {
        return st;
    }

    while (buffer_can_read(read_buffer)) {
        c   = buffer_read(read_buffer);
        st  = response_parser_feed(parser, c);

        if (st == response_error) {
            return st;
        }

        buffer_write(write_buffer, c);

        if (response_is_done(st, errored) || parser->first_line_consumed) {
            break;
        }

    }

    return st;
}
