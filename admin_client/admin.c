#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "include/send_request.h"
#include "include/receive_response.h"
#include "include/utils.h"

file_descriptor socket_fd;

struct parse_action {
    admin_status status;
    cmd_status (* function)(const cmd c, const char * buffer_option, bool * quit_option_on);
};

static struct parse_action auth_action = {
        .status      = ST_AUTH,
        .function    = authenticate,
};

static struct parse_action trans_action = {
        .status      = ST_TRANS,
        .function    = transaction,
};

static struct parse_action * action_list[] = {
        &auth_action,
        &trans_action,
};

void handle_receive_msg(enum response_status * r_status) {
    ssize_t     recv_bytes;
    struct response    response;

    recv_bytes = receive_response(r_status, &response);

    if (recv_bytes < 0) {
        printf("An error occured while receiving message from proxy_pop3.\n");
        *r_status = ERROR_RECEIVING;
        return;
    }

    print_msg(*r_status, response);

    if (response.length > 0) {
        free(response.data);
    }
}

cmd_status assemble_req(int i, const char * buffer_option, const b_cmd command) {
    char    param[MAX_PARAM]    = {0};
    int     j                   = 0;

    if(buffer_option[i] != SPACE){
        return BAD_SINTAXIS;
    } else {
        i++;

        for (; j < MAX_PARAM && buffer_option[i] != NEW_LINE; i++) {
            param[j++] = buffer_option[i];
        }

        if (j == MAX_PARAM) {
            return PARAM_TOO_LONG;
        } else if (j == 0) {
            return PARAM_TOO_SHORT;
        }

        send_request_one_param(param, command);
    }

    return WELL_WRITTEN;
}

cmd_status authenticate(const cmd c, const char * buffer_option, bool * quit_option_on) {
    int         i = 0;
    cmd_status  cmd_status;

    switch (c) {
        case A:
            cmd_status = assemble_req(++i, buffer_option, A_CMD);
            break;
        case Q:
            i++;
            if (buffer_option[i] != NEW_LINE) {
                cmd_status = BAD_SINTAXIS;
                break;
            } else {
                *quit_option_on = true;
                send_request_without_param(Q_CMD);
                cmd_status = WELL_WRITTEN;
                break;
            }
        case HELP:
            show_menu_authentication();
            cmd_status = HELP_CMD;
            break;
        default:
            cmd_status = INEXISTENT_CMD;
            break;
    }

    return cmd_status;
}

cmd_status transaction(const cmd c, const char * buffer_option, bool * quit_option_on) {
    int         i = 0;
    cmd_status  cmd_status;

    switch (c) {
        case SET_T:
            cmd_status = assemble_req(++i, buffer_option, SET_T_CMD);
            break;
        case GET_T:
            i++;
            if (buffer_option[i] != NEW_LINE) {
                cmd_status = BAD_SINTAXIS;
                break;
            } else {
                send_request_without_param(GET_T_CMD);
                cmd_status = WELL_WRITTEN;
                break;
            }
        case SWITCH_T:
            i++;
            if (buffer_option[i] != NEW_LINE) {
                cmd_status = BAD_SINTAXIS;
                break;
            } else {
                send_request_without_param(SWITCH_T_CMD);
                cmd_status = WELL_WRITTEN;
                break;
            }
        case GET_ME:
            cmd_status = assemble_req(++i, buffer_option, GET_ME_CMD);
            break;
        case GET_MI:
            i++;
            if (buffer_option[i] != NEW_LINE) {
                cmd_status = BAD_SINTAXIS;
                break;
            } else {
                send_request_without_param(GET_MI_CMD);
                cmd_status = WELL_WRITTEN;
                break;
            }
        case ALLOW_MI:
            cmd_status = assemble_req(++i, buffer_option, ALLOW_MI_CMD);
            break;
        case FORBID_MI:
            cmd_status = assemble_req(++i, buffer_option, FORBID_MI_CMD);
            break;
        case Q:
            i++;
            if (buffer_option[i] != NEW_LINE) {
                cmd_status = BAD_SINTAXIS;
                break;
            } else {
                *quit_option_on = true;
                send_request_without_param(Q_CMD);
                cmd_status = WELL_WRITTEN;
                break;
            }
        case HELP:
            show_menu_transaction();
            cmd_status = HELP_CMD;
            break;
        default:
            cmd_status = INEXISTENT_CMD;
            break;
    }

    return cmd_status;
}


void parse_cmd_status(cmd_status cmd_status, enum response_status * r_status) {
    switch (cmd_status) {
        case BAD_SINTAXIS:
                printf("Incorrect sintax of command. Press HELP option (0) to display menu again.\n");
                *r_status = NOT_SEND;
                break;
        case INEXISTENT_CMD:
                printf("The entered command does not exist or it's invalid for current status.\n");
                *r_status = NOT_SEND;
                break;
        case PARAM_TOO_LONG:
                printf("The entered parameter is too long to be sent. Maximum of 99 characters.\n");
                *r_status = NOT_SEND;
                break;
        case PARAM_TOO_SHORT:
                printf("The entered parameter is too short to be sent. Please enter at least 1 character.\n");
                *r_status = NOT_SEND;
                break;
        case WELL_WRITTEN:
                handle_receive_msg(r_status);
                break;
        default:
            break;
    }
}

void communicate_with_proxy() {
    char buffer_option[MAX_BUFFER];
    cmd c;
    cmd_status cmd_status;
    struct parse_action * act;
    admin_status a_status               = ST_AUTH;
    bool running                        = true;
    bool quit_option_on                 = false;
    enum response_status r_status       = OK;

    show_menu_authentication();

    while (running) {
        printf("\nInsert a command to run on proxy_pop3: ");

        if (fgets(buffer_option, MAX_BUFFER, stdin) == NULL) {
            printf("An error occured while reading from STDIN.\n");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }

        /* TODO: HACER ESTO BIEN !!!!!!!!! SI HAY MAS OPCIONES NO FUNCA*/
        c           = (cmd) buffer_option[0];
        act         = action_list[a_status];
        cmd_status  = act->function(c, buffer_option, &quit_option_on);

        parse_cmd_status(cmd_status,&r_status);

        if (r_status == OK) {
            if (a_status == ST_AUTH ) {
                if (quit_option_on != true) {
                    a_status = ST_TRANS;
                    printf("Now you are authenticated!\n");
                    show_menu_transaction();
                }
            }

            if (quit_option_on == true) {
                running = false;
            }
        }

    }

    printf("See you later!\n");
}

