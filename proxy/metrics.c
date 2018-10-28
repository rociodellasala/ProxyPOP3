#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/metrics.h"

double metrics[METRICS_SIZE];

void initialize_metrics() {
    int i ;
    for(i = 0; i < METRICS_SIZE; i++){
        metrics[i] = 100;
    }
}

char * metric_get_name(unsigned char * number, int * index) {
    enum e_metrics metric;
    int in = atoi (number);

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
        default:
            return NULL;
    }
}

void metric_add_transfered_bytes(double curr_transfered_bytes) {
    metrics[TRANSFERED_BYTES] += curr_transfered_bytes;
}