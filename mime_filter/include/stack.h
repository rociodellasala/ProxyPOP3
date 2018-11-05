#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

struct stack {
    struct element *    last;
    int size;
};

struct element {
    struct element *    prev;
    void *              data;
};

struct stack * stack_init();

void * stack_push(struct stack *, void *);

void * stack_pop(struct stack *);

void * stack_peek(struct stack *);

void stack_destroy(struct stack *);

#endif
