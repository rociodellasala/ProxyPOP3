#ifndef PROXYPOP3_METRICS_H
#define PROXYPOP3_METRICS_H

struct metrics {
    unsigned int    concurrent_connections;
    unsigned int    historical_access;
    long long int   transferred_bytes;
};

typedef struct metrics * metrics;

extern metrics program_metrics;

#endif //PROXYPOP3_METRICS_H
