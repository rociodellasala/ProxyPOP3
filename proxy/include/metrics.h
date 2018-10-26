<<<<<<< HEAD
//
// Created by rocio on 24/10/18.
//

#ifndef PROXYPOP3_METRICS_H
#define PROXYPOP3_METRICS_H

=======
#ifndef PROXYPOP3_METRICS_H
#define PROXYPOP3_METRICS_H

struct metrics {
    unsigned int concurrent_connections;
    unsigned int historical_access;
    long long int transferred_bytes;
};

typedef struct metrics * metrics;

extern metrics program_metrics;


>>>>>>> f63ebf25f7c99555cb02535cd0d41dc4fef5c2bc
#endif //PROXYPOP3_METRICS_H
