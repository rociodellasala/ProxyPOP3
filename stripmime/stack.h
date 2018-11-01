#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

struct stack {
    struct element *last;
    int size;
};

struct element {
    struct element *prev;
    void *data;
};

struct stack * stack_init();

void * stack_push(struct stack *s, void * data);

void * stack_pop(struct stack *s);

void * stack_peek(struct stack *s);

void stack_destroy(struct stack *s);

#endif
