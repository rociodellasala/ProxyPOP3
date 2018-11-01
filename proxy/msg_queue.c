#include <stdio.h>
#include <stdlib.h>
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

struct msg_queue * new_queue() {
    struct msg_queue * new_q = malloc(sizeof(*new_q));

    if (new_q == NULL) {
        return NULL;
    }

    new_q->first = new_q->last = NULL;
    new_q->current    = NULL;
    new_q->size = 0;

    return new_q;
}

static struct queue_node * new_node(void * data) {
    struct queue_node * new_n = malloc(sizeof(*new_n));

    if (new_n == NULL) {
        return NULL;
    }

    new_n->data = data;
    new_n->next = NULL;

    return new_n;
}

void enqueue(struct msg_queue * queue, void * data) {
    struct queue_node * last = queue->last;

    if(data == NULL){
        return ;
    }
    if (last == NULL) {
        last = new_node(data);
        queue->first = queue->last = last;
        queue->current =  queue->first;
    } else {
        last->next = new_node(data);
        queue->last = last->next;
    }

    queue->size++;
}


bool is_empty(struct msg_queue * queue) {
    return queue->size == 0;
}

void * dequeue(struct msg_queue * queue) {
    if (is_empty(queue)) {
        return NULL;
    }

    struct queue_node *first = queue->first;
    void * ret = first->data;

    queue->first = first->next;
    free(first);
    queue->size--;

    if (queue->first == NULL) {
        queue->last = NULL;
        queue->current = NULL;
    }

    return ret;
}

void * peek_data(struct msg_queue * queue) {
    void * ret = NULL;

    if (queue->first != NULL) {
        ret = queue->first->data;
    }

    return ret;
}

void * queue_get_next(struct msg_queue * queue) {
    void * ret;

    if (queue->current != NULL) {
        ret = queue->current->data;
        queue->current = queue->current->next;
    } else {
        queue->current = queue->first;
        ret = NULL;
    }

    return ret;
}

void destroy_queue(struct msg_queue * queue) {
    struct queue_node * first = queue->first;
    struct queue_node * aux;

    puts("aca");

    while (first != NULL) {
        aux = first->next;
        free(first);
        first = aux;
    }

    free(queue);
}