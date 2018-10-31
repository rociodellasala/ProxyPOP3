
//
// Created by rocio on 31/10/18.
//

#include "include/client_request_utils.h"

#include <string.h>
#include "include/client_request_utils.h"
#include "include/client_parser_request.h"
#include "include/utils.h"

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
