#ifndef PROXYPOP3_METRICS_H
#define PROXYPOP3_METRICS_H

enum e_metrics {
    INST_CONCURRENT_CONNECTIONS = 0,
    MAX_CONCURRENT_CONNECTIONS,
    HISTORICAL_ACCESSES,
    TRANSFERED_BYTES,
    CURRENT_ADMIN_CONNECTED,
    METRICS_SIZE
};

extern double metrics[METRICS_SIZE];

void initialize_metrics();

char * metric_get_name(unsigned char *, int *);

void metric_add_transfered_bytes(double);

void metric_add_new_connection();

void metric_add_admin_connected();

void metric_remove_admin_connected();

void metric_remove_current_connection();

#endif //PROXYPOP3_METRICS_H
