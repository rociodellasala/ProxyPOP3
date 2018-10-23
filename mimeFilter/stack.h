#ifndef TPE_PROTOS_STACK_H
#define TPE_PROTOS_STACK_H

#include <stdbool.h>

struct stack * stack_new();

void * stack_push(struct stack *s, void * data);

void * stack_pop(struct stack *s);

void * stack_peek(struct stack *s);

bool stack_is_empty(struct stack *s);

int stack_size(struct stack *s);

void stack_destroy(struct stack *s);

#endif //TPE_PROTOS_STACK_H
