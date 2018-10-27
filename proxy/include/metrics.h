#ifndef PROXYPOP3_METRICS_H
#define PROXYPOP3_METRICS_H

enum e_metrics {
    INST_CONCURRENT_CONNECTIONS = 0,
    MAX_CONCURRENT_CONNECTIONS,
    HISTORICAL_ACCESSES,
    TRANSFERED_BYTES,
    METRICS_SIZE
};

extern double metrics[METRICS_SIZE];

void initialize_metrics();

char * metric_get_name(unsigned char *, int * index);

#endif //PROXYPOP3_METRICS_H
