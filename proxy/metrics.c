#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/metrics.h"

double metrics[METRICS_SIZE];

void initialize_metrics() {
    int i ;
    for(i = 0; i < METRICS_SIZE; i++){
        metrics[i] = 0;
    }
}

char * metric_get_name(const char * number, int * index) {
    enum e_metrics metric;
    int in = atoi(number);

    if (in >= METRICS_SIZE || in < 0) {
        *index = METRICS_SIZE;
        return NULL;
    }

    metric = (enum e_metrics) in;

    switch (metric) {
        case INST_CONCURRENT_CONNECTIONS:
            *index = INST_CONCURRENT_CONNECTIONS;
            return "instant concurrent connections";
        case MAX_CONCURRENT_CONNECTIONS:
            *index = MAX_CONCURRENT_CONNECTIONS;
            return "max concurrent connections";
        case HISTORICAL_ACCESSES:
            *index = HISTORICAL_ACCESSES;
            return "historial acceses";
        case TRANSFERED_BYTES:
            *index = TRANSFERED_BYTES;
            return "transfered bytes";
        case CURRENT_ADMIN_CONNECTED:
            *index = CURRENT_ADMIN_CONNECTED;
            return "current admins connected";
        case MAX_ADMIN_CONNECTED:
            *index = MAX_ADMIN_CONNECTED;
            return "max admins connected";
        default:
            return NULL;
    }
}

void metric_add_admin_connected(){
    metrics[CURRENT_ADMIN_CONNECTED] += 1;
    if(metrics[MAX_ADMIN_CONNECTED] < metrics[CURRENT_ADMIN_CONNECTED]) {
        metrics[MAX_ADMIN_CONNECTED] = metrics[CURRENT_ADMIN_CONNECTED];
    }
}

void metric_remove_admin_connected(){
    metrics[CURRENT_ADMIN_CONNECTED] -= 1;
}

void metric_add_transfered_bytes(double curr_transfered_bytes) {
    metrics[TRANSFERED_BYTES] += curr_transfered_bytes;
}

void metric_add_new_connection() {
    metrics[INST_CONCURRENT_CONNECTIONS] += 1;
    if(metrics[MAX_CONCURRENT_CONNECTIONS] < metrics[INST_CONCURRENT_CONNECTIONS]) {
        metrics[MAX_CONCURRENT_CONNECTIONS] = metrics[INST_CONCURRENT_CONNECTIONS];
    }
}

void metric_remove_current_connection() {
    metrics[INST_CONCURRENT_CONNECTIONS] -= 1;
    metrics[HISTORICAL_ACCESSES] += 1;
}

