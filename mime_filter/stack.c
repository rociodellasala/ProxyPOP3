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
    struct stack * new_stack = malloc(sizeof(*new_stack));

    if (new_stack == NULL) {
        return NULL;
    }

    new_stack->last = NULL;
    new_stack->size = 0;

    return new_stack;
}

void * stack_push(struct stack * stack, void * data) {
    if (data == NULL) {
        return NULL;
    }

    struct element * node = create_element(data);

    if (node == NULL) {
        return NULL;
    }

    if (stack->last == NULL) {
        stack->last = node;
    } else {
        struct element * prev_last = stack->last;
        stack->last = node;
        node->prev = prev_last;
    }

    stack->size++;

    return data;
}

void * stack_pop(struct stack * stack) {
    if (stack->size == 0) {
        return NULL;
    }

    void *              ret     = stack->last->data;
    struct element *    node    = stack->last;

    stack->last = stack->last->prev;

    free(node);

    stack->size--;

    return ret;
}

void * stack_peek(struct stack * stack) {
    if (stack->last == NULL) {
        return NULL;
    }

    return stack->last->data;
}

void stack_destroy(struct stack * stack) {

    struct element * curr = stack->last;
    struct element * aux;

    while (curr != NULL) {
        aux = curr->prev;
        free(curr);
        curr = aux;
    }

    free(stack);
}