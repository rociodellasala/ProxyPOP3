
<<<<<<< HEAD
#include "include/input_parser.h"
#include "include/deserializer.h"

typedef int file_descriptor;

void parse_admin_request(struct admin * admin) {
    int status;
    int rd_sz;
    file_descriptor admin_fd = admin->fd;
    request_admin * request = malloc(sizeof(request));
    unsigned char buffer[40];

    rd_sz = sctp_recvmsg(key->fd, buffer, 40, NULL,0,0,0);

    if (rd_sz <= 0) {
        printf("%s\n", strerror(errno));
    }

    deserialize_request(buffer, request);
}

static struct parse_action hello_action = {
        .status      = ST_EHLO,
        .action      = parse_hello,
        .args        = 0,
};

static struct parse_action user_action = {
        .status      = ST_USER,
        .action      = parse_user,
        .args        = 2,
};

static struct parse_action pass_action = {
        .status      = ST_PASS,
        .action      = parse_pass,
        .args        = 2,
};

static struct parse_action config_action = {
        .status      = ST_CONFIG,
        .action      = parse_config,
        .args        = 0,
};

static struct parse_action * action_list[] = {
        &hello_action,
        &user_action,
        &pass_action,
        &config_action,
};
=======
#include <stdlib.h>
#include <netinet/sctp.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "include/input_parser.h"
#include "include/deserializer.h"
#include "include/admin.h"
#include "include/serializer.h"
#include "include/request_admin.h"
#include "include/admin_actions.h"
#include "include/response_admin.h"


typedef int file_descriptor;

void authenticate(struct admin * data);
void transaction(struct admin * data);

struct parse_action {
    admin_status status;
    int (* function)(struct admin * admin);
    int args;
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

void authenticate(struct admin * admin){
    request_admin * r = admin->current_request;

    if(r->cmd == AUTH){
        if(check_password(r->data) == 1){
            admin->a_status = ST_TRANS;
        } else {
            admin->req_status = INCORRECT_PASS;
        }
    } else {
        admin->req_status = INCORRECT_COMMAND_STATUS;
    }

}


void transaction(struct admin * admin){
    request_admin * r = admin->current_request;
    switch(r->cmd){
        case SET_TRANSF:
            /* TODO: Algun chequeo necesario ? */
            parameters->filter_command->program_name = r->data ;
            break;
        case GET_TRANSF:
            admin->resp_data = parameters->filter_command->program_name;
            admin->resp_length = strlen(admin->resp_data);
            break;
        case SWITCH_TRANSF:
            if(parameters->filter_command->switch_program == 0){
                parameters->filter_command->switch_program = 1;
            } else {
                parameters->filter_command->switch_program = 0;
            }
            break;
        case GET_METRIC:
            return_metric(admin, r->data);
            break;
        case GET_MIME:
            admin->resp_data = parameters->filtered_media_types;
            admin->resp_length = strlen(admin->resp_data);
            break;
        case ALLOW_MIME:
            //allow_mime()
            break;
        case FORBID_MIME:
            //forbid_mime()
            break;
        case QUIT:
            admin->quit = 1;
            break;
        default:
            admin->req_status = INCORRECT_COMMAND_STATUS;
            break;
    }
    
}

void parse_req_commands(struct admin * admin){
    struct parse_action * act = action_list[admin->a_status-1];
    act->function(admin);

    switch (admin->req_status) {
        case INCORRECT_PASS:
            admin->resp_data = "Incorrect password. Could not authenticate.";
            admin->resp_length = strlen(admin->resp_data);
            break;
        case INCORRECT_COMMAND_STATUS:
            if(admin->a_status == ST_AUTH) {
                admin->resp_data = "Incorrect command for current status: First you have to authenticate.";
            } else {
                admin->resp_data = "Incorrect command for current status: You are already authenticated.";
            }
            admin->resp_length = strlen(admin->resp_data);
            break;
        case COULD_NOT_READ_REQUEST:
            admin->resp_data = "Server Error. Please try again.";
            admin->resp_length = strlen(admin->resp_data);
            break;
        default:
            break;
    }

}

void parse_admin_request(struct admin * admin) {
    int read_bytes;
    unsigned char buffer[100];
    request_admin * request     = malloc(sizeof(*request));

    read_bytes = sctp_recvmsg(admin->fd, buffer, 100, NULL, 0, 0, 0);

    if (read_bytes <= 0) {
        admin->req_status = COULD_NOT_READ_REQUEST;
        admin->resp_data = "Server Error. Please try again.";
        admin->resp_length = strlen(admin->resp_data);
    } else {
        deserialize_request(buffer, request);
        admin->current_request = request;
        parse_req_commands(admin);
    }

}

>>>>>>> f63ebf25f7c99555cb02535cd0d41dc4fef5c2bc
