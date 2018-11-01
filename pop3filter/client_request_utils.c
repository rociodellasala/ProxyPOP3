#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/client_request_utils.h"
#include "include/client_parser_request.h"
#include "include/utils.h"
#include "include/client_request.h"

#define ERROR (-1)

enum request_state check_request_against_current_session_status(enum pop3_session_state state, struct pop3_request * request){
    enum request_state ret_state = request_error_cmd_incorrect_status;

    if (state == POP3_AUTHORIZATION) {
        switch (request->cmd->id) {
            case user:
            case pass:
            case capa:
            case quit:
                ret_state = request_done;
                break;
            default:
                ret_state = request_error_cmd_incorrect_status;
                break;

        }
    } else if (state == POP3_TRANSACTION) {
        switch (request->cmd->id) {
            case retr:
            case list:
            case stat:
            case dele:
            case noop:
            case top:
            case rset:
            case capa:
            case quit:
                ret_state = request_done;
                break;
            default:
                ret_state = request_error_cmd_incorrect_status;
                break;

        }
    }

    return ret_state;
}

void send_error_request(enum request_state state, char *name, file_descriptor fd) {
    char * dest = NULL;
    char * msg = NULL;

    switch (state) {
        case request_error_inexistent_cmd:
            msg = "-ERR: Unknown command ";
            dest = append_cmd(dest, msg, name);
            break;
        case request_error_cmd_too_long:
            dest = "-ERR: Command too long\r\n";
            break;
        case request_error_param_too_long:
            dest = "-ERR: Parameter too long\r\n";
            break;
        case request_error_cmd_incorrect_status:
            dest = "-ERR: Command not correct for current session status\n";
            break;
        default:
            break;
    }

    send(fd, dest, strlen(dest), 0);
    free(dest);
}

int request_marshall(struct pop3_request * request, buffer * buffer) {
    size_t n;
    uint8_t *  buff = buffer_write_ptr(buffer, &n);

    const char * cmd    = request->cmd->name;
    char * args         = request->args;
    unsigned int i = (unsigned int) strlen(cmd);
    unsigned int j;
    unsigned int total;

    if (args == NULL) {
        j       = 0;
        total   = i + j + 2;
    } else {
        j       = (unsigned int) strlen(args);
        total   = i + j + 3;
    }

    if(n < total) {
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


int request_to_buffer(buffer * buffer, bool pipelining, struct pop3_request * pop3_request, struct msg_queue * queue) {
    char * msg = "Error while copying request to buffer\n";

    if (pipelining == false) { // si el server no soporta pipelining solo copio la primer request
        pop3_request = peek_data(queue);
        if (request_marshall(pop3_request, buffer) == -1) {
            fprintf(stderr, "%s", msg);
            perror("");
            return ERROR;
        }
    } else { // si el server soporta pipelining entonces copio al buffer todas las request encoladas
        while ((pop3_request = queue_get_next(queue)) != NULL) {
            if (request_marshall(pop3_request, buffer) == -1) {
                fprintf(stderr, "%s", msg);
                perror("");
                return ERROR;
            }
        }
    }

    return 0;
}