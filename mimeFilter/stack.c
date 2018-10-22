#include <stdlib.h>

#include "stack.h"

struct stack {
    struct stack_node *first, *last;
    int size;
};

struct stack_node {
    void *data;
    struct stack_node *prev;
};

struct stack * stack_new() {
    struct stack *ret = malloc(sizeof(*ret));

    if (ret == NULL) {
        return NULL;
    }

    ret->first = ret->last = NULL;
    ret->size = 0;

    return ret;
}

static struct stack_node * new_node(void * data) {
    struct stack_node * node = malloc(sizeof(*node));

    if (node == NULL) {
        return NULL;
    }

    node->data = data;
    node->prev = NULL;

    return node;
}

void * stack_push(struct stack *s, void * data) {
    if (data == NULL) {
        return NULL;
    }

    struct stack_node *node = new_node(data);

    if (node == NULL) {
        return NULL;
    }

    if (s->first == NULL) {
        s->first = s->last = node;
    } else {
        struct stack_node * prev_last = s->last;
        s->last = node;
        node->prev = prev_last;
    }

    s->size++;

    return data;
}

void * stack_pop(struct stack *s) {
    if (stack_is_empty(s)) {
        return NULL;
    }

    void * ret = s->last->data;
    struct stack_node *node = s->last;

    s->last = s->last->prev;

    free(node);

    if (s->last == NULL) {
        s->first = NULL;
    }

    s->size--;

    return ret;
}

void * stack_peek(struct stack *s) {
    if (s->last == NULL) {
        return NULL;
    }

    return s->last->data;
}

bool stack_is_empty(struct stack *s) {
    return s->size == 0;
}

int stack_size(struct stack *s) {
    return s->size;
}

void stack_destroy(struct stack *s) {
    struct stack_node *node = s->last;
    struct stack_node *aux;

    while (node != NULL) {
        aux = node->prev;
        free(node);
        node = aux;
    }

    free(s);
}