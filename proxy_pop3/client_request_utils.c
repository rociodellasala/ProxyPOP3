#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/client_request_utils.h"
#include "include/client_parser_request.h"
#include "include/utils.h"
#include "include/client_request.h"
#include "include/pop3nio.h"
#include "include/logs.h"

void send_error_request(enum request_state state, file_descriptor fd) {
    char * dest = NULL;

    switch (state) {
        case request_error_inexistent_cmd:
            dest = "-ERR Unknown command for proxy.\r\n";
            break;
        case request_error_cmd_too_long:
            dest = "-ERR Command too long: Max 4 characters.\r\n";
            break;
        case request_error_param_too_long:
            dest = "-ERR Parameter too long: Max 40 characters per argument.\r\n";
            break;
        case request_error_too_many_params:
            dest = "-ERR Too many parameters for current command.\r\n";
            break;
        default:
            break;
    }

    send(fd, dest, strlen(dest), 0);
}

enum pop3_state process_request(struct selector_key * key, struct request_st * request) {
    enum pop3_state stm_next_status = REQUEST;

    if (request->request_parser.state >= request_error_inexistent_cmd) {
        // si hay un error le envio un mensaje y vuelvo a intentar leer una nueva request
        send_error_request(request->request_parser.state, ATTACHMENT(key)->client_fd);
        request_parser_reset(&request->request_parser);
        return REQUEST;
    }

    // si no hay error entonces armo la request y la encolo

    struct pop3_request * req = new_request(request->request.cmd, request->request.args);
    
    if (req == NULL) {
        log_request(false, (char *) req->cmd->name, req->args, "REQUEST not send");
        return ERROR;
    } else {
        log_request(true, (char *) req->cmd->name, req->args, "REQUEST send");
    }

    enqueue(ATTACHMENT(key)->session.request_queue, req);
    request_parser_reset(&request->request_parser);

    if (!buffer_can_read(request->read_buffer)) {
        // si no se puede leer mas entonces intento escribir la request en el origen
        selector_status ss = SELECTOR_SUCCESS;
        ss |= selector_set_interest_key(key, OP_NOOP);
        ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_WRITE);

        stm_next_status = SELECTOR_SUCCESS == ss ? stm_next_status : ERROR;
    }

    return stm_next_status;
}

int request_marshall(struct pop3_request * request, buffer * buffer) {
    size_t n;
    uint8_t *  buff = buffer_write_ptr(buffer, &n);

    const char *    cmd    = request->cmd->name;
    char *          args   = request->args;
    unsigned int    i      = (unsigned int) strlen(cmd);
    unsigned int    j;
    unsigned int    total;

    if (args == NULL) {
        j       = 0;
        total   = i + j + 2;
    } else {
        j       = (unsigned int) strlen(args);
        total   = i + j + 3;
    }

    if (n < total) {
        return -1;
    }

    memcpy(buff, cmd, i);
    buffer_write_adv(buffer, i);

    if (args != NULL) {
        buffer_write(buffer, ' ');
        buff = buffer_write_ptr(buffer, &n);
        memcpy(buff, args, j);
        buffer_write_adv(buffer, j);
    }

    buffer_write(buffer, '\r');
    buffer_write(buffer, '\n');

    return total;
}


struct pop3_request * request_to_buffer(buffer * buffer, bool pipelining, struct pop3_request * pop3_request, struct msg_queue * queue) {
    if (pop3_request == NULL) {
        if (pipelining == false) {
            pop3_request = peek_data(queue);

            if (request_marshall(pop3_request, buffer) == -1) {
                return pop3_request;
            }

        } else {
            while ((pop3_request = queue_get_next(queue)) != NULL) {
                if (request_marshall(pop3_request, buffer) == -1) {
                    return pop3_request;
                }
            }
        }
    } else {
        request_marshall(pop3_request, buffer);

        while ((pop3_request = queue_get_next(queue)) != NULL) {
            if (request_marshall(pop3_request, buffer) == -1) {
                return pop3_request;
            }
        }
    }

    return NULL;
}