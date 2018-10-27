#include <string.h>
#include <malloc.h>
#include <netinet/sctp.h>
#include "include/admin.h"
#include "include/metrics.h"
#include "include/admin_parser.h"
#include "include/input_parser.h"

int check_password(const char * pass) {
    const char * password = "1234";
    if (strcmp(pass, password) == 0) {
        return 1;
    } else {
        return 0;
    }
}

void switch_transformation_program(struct admin * admin){
    bool * pointer = &(parameters->filter_command->switch_program);
    *pointer = !(*pointer);

    if(*pointer == false){
        admin->resp_data = "OFF";
    } else {
        admin->resp_data = "ON";
    }

    admin->resp_length = (unsigned int) strlen((const char *) admin->resp_data);
}

void return_metric(struct admin * admin, const char * data) {
    metrics metrics = program_metrics;
    char * aux = malloc(30 * sizeof(char)); /*TODO:CAMBIAR*/

    if (strcmp(data, "cc") == 0) {
        sprintf(aux, "%d", metrics->concurrent_connections);
    } else if (strcmp(data, "ha") == 0) {
        sprintf(aux, "%d", metrics->historical_access);
    } else if (strcmp(data, "tb") == 0) {
        sprintf(aux, "%d", (int) metrics->transferred_bytes);
    } else {
        admin->req_status =  INCORRECT_METRIC;
        return;
    }

    admin->resp_data = aux;
    admin->resp_length = (unsigned int) strlen((const char *) admin->resp_data);
}

/* TODO: creo que hay que llamar a lo que hicimos en el tp2 para saber si el mime no esta ya incluido
void forbid_mime(request_admin * request, int * status, char * media_types){
    int ret; //check_mime_not_in(request->data, media_types);
    if(ret == -1) {
        //add_mime();
    } else {
        *status = 0;
    }
}

void allow_mime(request_admin * request, int * status, char * media_types){
    int ret; //check_mime_not_in(request->data, media_types);
    if(ret == -1) {
        //remove_mime();
    } else {
        *status = 0;
    }
}

*/

void quit(struct admin * admin) {
    send_response_without_data(admin, 1);
    close(admin->fd);
}