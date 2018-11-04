#include <string.h>
#include <malloc.h>
#include <netinet/sctp.h>
#include "include/admin.h"
#include "include/metrics.h"
#include "include/admin_parser.h"
#include "include/input_parser.h"
#include "include/utils.h"
#include "include/admin.h"

int check_password(const char * pass) {
    const char * password = "1234";

    if (strcmp(pass, password) == 0) {
        return 1;
    } else {
        return 0;
    }
}

void switch_transformation_program(struct admin * admin) {
    bool * pointer = &(parameters->filter_command->switch_program);

    *pointer = !(*pointer);

    if (*pointer == false) {
        admin->resp_data = "OFF";
    } else {
        admin->resp_data = "ON";
    }

    admin->resp_length = (unsigned int) strlen((const char *) admin->resp_data);
}

void return_metric(struct admin * admin, const char * data) {
    int     index;
    int     size;
    char *  resp;
    char *  value;
    char *  name;
    double  metric;

    name = metric_get_name(data, &index);
    
    if (index >= METRICS_SIZE ){
        admin->req_status =  INCORRECT_METRIC;
        return;
    }

    metric  = metrics[index];
    size    = get_int_len((int) metric);
    
    value = malloc(size *  sizeof(char *));
    sprintf(value, "%.0f", metric);
    
    resp = malloc((strlen(name) + 2 + size ) * sizeof(char *));
    sprintf(resp, "%s: %s", name, value);

    admin->resp_data    = resp;
    admin->resp_length  = (unsigned int) strlen((const char *) admin->resp_data);

    free(value);
    free(resp);
}


void forbid_mime(struct request_admin * request, enum parse_req_status* status){

    char* type = malloc(strlen((const char *) request->data) * sizeof(char));
    char* subtype = malloc(strlen((const char *) request->data) * sizeof(char));
    if(check_mime_format((char *) request->data, &type, &subtype) == -1){
        *status = FORBID_ERROR; //error
        return;
    }
    bool already_there = find_mime(parameters->filtered_media_types, type, subtype);
    if(already_there){
        *status = MIME_ALREADY_FORBID; //ya estaba
        return;
    }


    int forbid_status = forbid_new(type, subtype, parameters->filtered_media_types);
    if(forbid_status != 0){
        *status = FORBID_ERROR; //error
        return;
    }

    *status = REQ_PARSE_OK;
}

void allow_mime(struct request_admin * request, enum parse_req_status* status){
        char* type = malloc(strlen((const char *) request->data) * sizeof(char));
        char* subtype = malloc(strlen((const char *) request->data) * sizeof(char));
        if(check_mime_format((char *) request->data, &type, &subtype) == -1){
            *status = ALLOW_ERROR; //error
            return;
        }
        bool is_there = find_mime(parameters->filtered_media_types, type, subtype);
        if(!is_there){


            if(strcasecmp(subtype, "*") == 0){
                int allow_status = allow_type(type, subtype, parameters->filtered_media_types);
                if(allow_status == -1){
                    *status = ALLOW_ERROR;
                    return;
                }
                *status = REQ_PARSE_OK;
                return;
            }
            *status = MIME_ALREADY_ALLOWED; // no estaba en la lista de prohibidos
            return;
        }


        int allow_status = allow_type(type, subtype, parameters->filtered_media_types);
        if(allow_status == -2){
            *status = CANNOT_ALLOW_BECAUSE_WILDCARD; //error
            return;
        }else if(allow_status == -1){
            *status = ALLOW_ERROR;
            return;
        }

        *status = REQ_PARSE_OK;
}



void quit_admin(struct admin * admin) {
    send_response_without_data(admin, 1);
    close(admin->fd);
}