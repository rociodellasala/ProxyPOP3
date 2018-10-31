#ifndef PROXYPOP3_QUEUE_H
#define PROXYPOP3_QUEUE_H

#include <stdbool.h>

/* */
struct queue * new_queue(void);

/* */
void enqueue(struct queue *, void *);

/* */
void * dequeue(struct queue *);

/* */
bool is_empty(struct queue *);

/* */
void * peek_data(struct queue *);

/* */
void * queue_get_next(struct queue *);

/* */
void destroy_queue(struct queue *);

#endif //PROXYPOP3_QUEUE_H
