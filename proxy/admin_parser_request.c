
#include <stdlib.h>
#include <netinet/sctp.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "include/input_parser.h"
#include "include/deserializer.h"
#include "include/admin.h"
#include "include/admin_serializer.h"
#include "include/admin_request.h"
#include "include/admin_actions.h"
#include "include/admin_response.h"
#include "include/utils.h"

void parse_action(struct admin * admin) {
    request_admin * r = admin->current_request;
    
    switch (r->cmd) {
        case A_CMD:
            if(check_password((const char *) r->data) == 1){
                admin->a_status = ST_CONNECTED;
            } else {
                admin->req_status = INCORRECT_PASS;
            }
            break;
        case SET_T_CMD:
            /* TODO: Algun chequeo necesario ? */
            parameters->filter_command->program_name = r->data;
            break;
        case GET_T_CMD:
            admin->resp_data = (char *) parameters->filter_command->program_name;
            admin->resp_length = strlen((const char *) admin->resp_data);
            break;
        case SWITCH_T_CMD:
            switch_transformation_program(admin);
            break;
        case GET_ME_CMD:
            return_metric(admin, (const char *) r->data);
            break;
        case GET_MI_CMD:
            admin->resp_data = parameters->filtered_media_types;
            admin->resp_length = strlen((const char *) admin->resp_data);
            break;
        case ALLOW_MI_CMD:
            /* TODO: Ale cuidado cuando agregues un mime a la lista porque las respuestas tienen un maximo de
             * 100 caracteres para la parte de data y entonces si despues agregamos tantos que eso se sobrepasa
             * y hacemos el comando 6 para pedir, va a explotar toodo */
            //allow_mime()
            break;
        case FORBID_MI_CMD:
            //forbid_mime()
            break;
        case Q_CMD:
            admin->quit = 1;
            break;
        default:
            break;
    }
    
}

void parse_req_commands(struct admin * admin){
    parse_action(admin);

    switch (admin->req_status) {
        case INCORRECT_PASS:
            admin->resp_data = "Incorrect password. Could not authenticate.";
            admin->resp_length = strlen((const char *) admin->resp_data);
            break;
        case INCORRECT_METRIC:
            admin->resp_data = "Incorrect metric. It does not exists.";
            admin->resp_length =  strlen((const char *) admin->resp_data);
            break;
        default:
            break;
    }

}

/* TODO: VER MALLOC QUE SE PIERDE VALGRIND */
int parse_admin_request(struct admin * admin) {
    int read_bytes;
    unsigned char buffer[MAX_ADMIN_BUFFER];
    request_admin * request     = malloc(sizeof(*request));

    read_bytes = sctp_recvmsg(admin->fd, buffer, MAX_ADMIN_BUFFER, NULL, 0, 0, 0);

    if (read_bytes <= 0) {
        admin->req_status = COULD_NOT_READ_REQUEST;
        admin->resp_data = "Server Error while reading. Please try again.";
        admin->resp_length = strlen((const char *) admin->resp_data);
    } else {
        deserialize_request(buffer, request);
        admin->current_request = request;
        parse_req_commands(admin);
    }
    if (admin->req_status == COULD_NOT_READ_REQUEST) {
        return -1;
    }

}

