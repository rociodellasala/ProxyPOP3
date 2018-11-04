#ifndef PROXYPOP3_QUEUE_H
#define PROXYPOP3_QUEUE_H

#include <stdbool.h>

struct queue_node {
    void *                  data;
    struct queue_node *     next;
};

struct msg_queue {
    struct queue_node *     first,
            *     last,
            *     current;
    int 				    size;
};

/* */
struct msg_queue * new_queue(void);

/* */
void enqueue(struct msg_queue *, void *);

/* */
void * dequeue(struct msg_queue *);

/* */
bool is_empty(struct msg_queue *);

/* */
void * peek_data(struct msg_queue *);

/* */
void * queue_get_next(struct msg_queue *);

/* */
void destroy_queue(struct msg_queue *);

#endif //PROXYPOP3_QUEUE_H
