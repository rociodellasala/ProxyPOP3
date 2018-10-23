#include <stdio.h>
#include <stdlib.h>
#include "include/queue.h"

struct queue {
    struct queue_node 	*first, *last;
    int 				size;
};

struct queue_node {
    void *data;
    struct queue_node *next;
};

struct queue * new_queue() {
    struct queue *ret = malloc(sizeof(*ret));

    if (ret == NULL) {
        return NULL;
    }

    ret->first = ret->last = NULL;
    ret->size = 0;

    return ret;
}

static struct queue_node * new_node(void *data) {
    struct queue_node *ret = malloc(sizeof(*ret));

    if (ret == NULL) {
        return NULL;
    }

    ret->data = data;
    ret->next = NULL;

    return ret;
}

//todo checkear null
void enqueue(struct queue *q, void *data) {
    struct queue_node *last = q->last;

    if (last == NULL) {
        last = new_node(data);	// queue vacia
        q->first = q->last = last;
    } else {
        last->next = new_node(data);
        q->last = last->next;
    }

    q->size++;
}

void * dequeue(struct queue *q) {
    if (is_empty(q)) {
        return NULL;
    }

    struct queue_node *first = q->first;
    void * ret = first->data;

    q->first = first->next;
    free(first);
    q->size--;

    if (q->first == NULL) {
        q->last = NULL;
    }

    return ret;
}

bool is_empty(struct queue *q) {
    return q->size == 0;
}

int size(struct queue *q) {
    return q->size;
}

void destroy_queue(struct queue *q) {
    //TODO free nodes
    free(q);
}