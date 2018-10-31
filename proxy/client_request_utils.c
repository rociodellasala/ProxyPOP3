#include <string.h>
#include <stdio.h>
#include "include/client_request_utils.h"
#include "include/client_parser_request.h"
#include "include/utils.h"
#include "include/client_request.h"

#define ERROR -1

enum request_state check_request_against_current_session_status(enum pop3_session_state state, struct pop3_request * request){
    enum request_state ret_state;

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

void send_error_request(enum request_state state, char * name, file_descriptor fd) {
    char * dest;
    char * msg;

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
}

int request_to_buffer(buffer * buffer, bool pipelining, struct pop3_request * pop3_request, struct queue * queue) {
    char * msg = "Error while copying request to buffer";
    if (pipelining == false) { // si el server no soporta pipelining solo copio la primer request
        pop3_request = peek_data(queue);
        if (request_marshall(pop3_request, buffer) == -1) {
            fprintf(stderr, msg);
            return ERROR;
        }
    } else { // si el server soporta pipelining entonces copio al buffer todas las request encoladas
        while ((pop3_request = queue_get_next(queue)) != NULL) {
            if (request_marshall(pop3_request, buffer) == -1) {
                fprintf(stderr, msg);
                return ERROR;
            }
        }
    }
}