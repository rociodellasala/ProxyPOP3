#ifndef PROXYPOP3_QUEUE_H
#define PROXYPOP3_QUEUE_H

#include <stdbool.h>

/* */
struct queue * new_queue(void);

/* */
void enqueue(struct queue *q, void *);

/* */
void * dequeue(struct queue *);

/* */
bool is_empty(struct queue *);

/* */
int size(struct queue *);

/* */
void destroy_queue(struct queue *);

#endif //PROXYPOP3_QUEUE_H
