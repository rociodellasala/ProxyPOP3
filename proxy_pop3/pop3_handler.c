#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "include/pop3nio.h"
#include "include/stm.h"
#include "include/metrics.h"
#include "include/pop3_session.h"

/**
 * Handlers top level de la conexión pasiva.
 * son los que emiten los eventos a la maquina de estados.
 */
void pop3_done(struct selector_key * key) {
    unsigned int i;

    const int fds[] = {
            ATTACHMENT(key)->client_fd,
            ATTACHMENT(key)->origin_fd,
    };

    metric_remove_current_connection();

    for (i = 0; i < N(fds); i++) {
        if (fds[i] != -1) {
            if (SELECTOR_SUCCESS != selector_unregister_fd(key->s, fds[i])) {
                abort();
            }
            close(fds[i]);
        }
    }

    pop3_session_close(&ATTACHMENT(key)->session);
}


void pop3_read(struct selector_key * key) {
    struct state_machine * stm  = &ATTACHMENT(key)->stm;
    const enum pop3_state  st   = (enum pop3_state) stm_handler_read(stm, key);

    if (ERROR == st || DONE == st) {
        pop3_done(key);
    }
}

void pop3_write(struct selector_key * key) {
    struct state_machine * stm  = &ATTACHMENT(key)->stm;
    const enum pop3_state  st   = (enum pop3_state)stm_handler_write(stm, key);

    if (ERROR == st || DONE == st) {
        pop3_done(key);
    }

}

void pop3_block(struct selector_key * key) {
    struct state_machine * stm   = &ATTACHMENT(key)->stm;
    const enum pop3_state  st    = (enum pop3_state) stm_handler_block(stm, key);

    if (ERROR == st || DONE == st) {
        pop3_done(key);
    }
}

void pop3_close(struct selector_key * key) {
    pop3_destroy(ATTACHMENT(key));
}
