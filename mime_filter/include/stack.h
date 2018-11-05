#ifndef PROXYPOP3_STACK_H
#define PROXYPOP3_STACK_H

#include <stdbool.h>

/* Estructura de un stack. Tenemos el tamaño y un puntero al ultimo elemento. */
struct stack {
    struct element *    last;
    int size;
};

/* Cada elemento tiene un puntero al anterior y los datos que debe almacenar. */
struct element {
    struct element *    prev;
    void *              data;
};

/* Inicialización de la estructura. */
struct stack * stack_init();

/* Pushea un elemento al stack con los datos que recibe. */
void * stack_push(struct stack *, void *);

/* Retorna los datos del ultimo elemento y lo saca del stack. */
void * stack_pop(struct stack *);

/* Retorna los datos del ultimo elemento y sin sacarlo del stack. */
void * stack_peek(struct stack *);

/* Libera el stack y todos sus elementos. */
void stack_destroy(struct stack *);

#endif
