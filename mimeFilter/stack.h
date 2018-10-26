#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

struct stack * stack_init();

static struct element * create_element(void * data);

void * stack_push(struct stack *s, void * data);

void * stack_pop(struct stack *s);

void * stack_peek(struct stack *s);

void stack_destroy(struct stack *s);

#endif
