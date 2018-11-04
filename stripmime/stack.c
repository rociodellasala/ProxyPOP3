#include <stdlib.h>

#include "include/stack.h"


static struct element * create_element(void * data) {
    struct element * node = malloc(sizeof(*node));

    if (node == NULL) {
        return NULL;
    }

    node->data = data;
    node->prev = NULL;

    return node;
}

struct stack * stack_init() {

    struct stack *s = malloc(sizeof(*s));

    if (s == NULL) {
        return NULL;
    }

    s->last = NULL;
    s->size = 0;

    return s;
}



void * stack_push(struct stack *s, void * data) {
    if (data == NULL) {
        return NULL;
    }

    struct element *node = create_element(data);

    if (node == NULL) {
        return NULL;
    }

    if (s->last == NULL) {
        s->last = node;
    } else {
        struct element * prev_last = s->last;
        s->last = node;
        node->prev = prev_last;
    }

    s->size++;

    return data;
}

void * stack_pop(struct stack *s) {
    if (s->size == 0) {
        return NULL;
    }

    void * ret = s->last->data;
    struct element *node = s->last;

    s->last = s->last->prev;

    free(node);

    s->size--;

    return ret;
}

void * stack_peek(struct stack *s) {
    if (s->last == NULL) {
        return NULL;
    }

    return s->last->data;
}

void stack_destroy(struct stack *s) {

    struct element *curr = s->last;
    struct element *aux;

    while (curr != NULL) {
        aux = curr->prev;
        free(curr);
        curr = aux;
    }

    free(s);
}