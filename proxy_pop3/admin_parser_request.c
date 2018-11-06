#include <stdlib.h>
#include <netinet/sctp.h>
#include <string.h>
#include "include/input_parser.h"
#include "include/admin_deserializer.h"
#include "include/admin.h"
#include "include/admin_actions.h"
#include "include/filter_list.h"

void parse_action(struct admin * admin) {
    struct request_admin * r = admin->current_request;
    
    switch (r->cmd) {
        case A_CMD:
            if (check_password((const char *) r->data) == 1) {
                admin->a_status = ST_CONNECTED;
            } else {
                admin->req_status = INCORRECT_PASS;
            }
            break;
        case SET_T_CMD:
            if(strstr((char *)r->data,"stripmime") != NULL){
                admin->resp_data = "You chose stripmime. Remember to forbid at least one media type, otherwise transformation will fail.";
                admin->resp_length = strlen((const char *) admin->resp_data);
                admin->a_status = 1;
            }
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
            //TODO ojo si data es muy largo. 
            admin->resp_data = get_forbidden_types(parameters->filtered_media_types);
            admin->resp_length = strlen((const char *) admin->resp_data);
            break;
        case ALLOW_MI_CMD:
            allow_mime(r, &admin->req_status);     
            break;
        case FORBID_MI_CMD:
            forbid_mime(r, &admin->req_status);
            break;
        case Q_CMD:
            admin->quit = true;
            break;
        default:
            break;
    }
}

void parse_req_commands(struct admin * admin) {
    if (admin->current_request->version > VERSION) {
        admin->req_status = VERSION_UNSOPPORTED;
    } else {
        parse_action(admin);
    }

    switch (admin->req_status) {
        case INCORRECT_PASS:
            admin->resp_data    = "Incorrect password. Could not authenticate.";
            admin->resp_length  = strlen((const char *) admin->resp_data);
            break;
        case INCORRECT_METRIC:
            admin->resp_data    = "Incorrect metric. It does not exists.";
            admin->resp_length  =  strlen((const char *) admin->resp_data);
            break;
        case MIME_ALREADY_FORBID:
            admin->resp_data    = "MIME type was already forbidden.";
            admin->resp_length  =  strlen((const char *) admin->resp_data);
            break;
        case MIME_ALREADY_ALLOWED:
            admin->resp_data    = "MIME type was already allowed.";
            admin->resp_length  =  strlen((const char *) admin->resp_data);
            break;
        case FORBID_ERROR:
            admin->resp_data    = "Error trying to forbid MIME type.";
            admin->resp_length  =  strlen((const char *) admin->resp_data);
            break;
        case ALLOW_ERROR:
            admin->resp_data    = "Error trying to allow MIME type.";
            admin->resp_length  =  strlen((const char *) admin->resp_data);
            break;
        case CANNOT_ALLOW_BECAUSE_WILDCARD:
            admin->resp_data    = "You cannot allow a specific subtype if you previously forbid the whole type.";
            admin->resp_length  =  strlen((const char *) admin->resp_data);
            break;
        case VERSION_UNSOPPORTED:
            admin->resp_data    = "Version unsopported.";
            admin->resp_length  =  strlen((const char *) admin->resp_data);
        default:
            break;
    }
}

/* TODO: VER MALLOC QUE SE PIERDE VALGRIND */
int parse_admin_request(struct admin * admin) {
    int                     read_bytes;
    unsigned char           buffer[MAX_ADMIN_BUFFER];
    struct request_admin *  request     = malloc(sizeof(*request));

    read_bytes = sctp_recvmsg(admin->fd, buffer, MAX_ADMIN_BUFFER, NULL, 0, 0, 0);

    if (read_bytes <= 0) {
        admin->req_status   = COULD_NOT_READ_REQUEST;
        admin->resp_data    = "Server Error while reading. Please try again.";
        admin->resp_length  = strlen((const char *) admin->resp_data);
    } else {
        deserialize_request(buffer, request);
        admin->current_request = request;
        parse_req_commands(admin);
    }

    if (admin->req_status == COULD_NOT_READ_REQUEST) {
        return -1;
    }

    return 0;
}

