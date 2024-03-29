#ifndef PROXYPOP3_STM_H
#define PROXYPOP3_STM_H

/**
 * stm.c - pequeño motor de maquina de estados donde los eventos son los
 *         del selector.c
 *
 * La interfaz es muy simple, y no es un ADT.
 *
 * Los estados se identifican con un número entero (típicamente proveniente de
 * un enum).
 *
 *  - El usuario instancia un `struct state_machine'
 *  - Describe la maquina de estados:
 *      - describe el estado inicial en `initial'
 *      - todos los posibles estados en `states' (el orden debe coincidir con
 *        el identificador)
 *      - describe la cantidad de estados en `states'.
 *
 * Provee todas las funciones necesitadas en un `struct fd_handler'
 * de selector.c.
 */

struct state_machine {
    // declaración de cual es el estado inicial
    int                             initial;

    //declaracion de los estados
    const struct state_definition * states;
    // cantidad de estados
    int                             max_state;

    // estado actual
    const struct state_definition * current;
};

struct selector_key * key;

/**
 * Definición de un estado de la máquina de estados
 */
struct state_definition {
    // identificador del estado
    int state;

    // ejecutado al arribar al estado
    void     (* on_arrival)    (struct selector_key * key);
    // ejecutado al salir del estado
    void     (* on_departure)  (struct selector_key * key);
    // ejecutado cuando hay datos disponibles para ser leidos
    int      (* on_read_ready) (struct selector_key * key);
    // ejecutado cuando hay datos disponibles para ser escritos
    int      (* on_write_ready)(struct selector_key * key);
    // ejecutado cuando hay una resolución de nombres lista
    int      (* on_block_ready)(struct selector_key * key);
};


/* Inicializa el la máquina */
void stm_init(struct state_machine *);

/* Indica que ocurrió el evento read. retorna nuevo id de nuevo estado. */
int stm_handler_read(struct state_machine *, struct selector_key *);

/* Indica que ocurrió el evento write. retorna nuevo id de nuevo estado. */
int stm_handler_write(struct state_machine *, struct selector_key *);

/* Indica que ocurrió el evento block. retorna nuevo id de nuevo estado. */
int stm_handler_block(struct state_machine *, struct selector_key *);


#endif //PROXYPOP3_STM_H
