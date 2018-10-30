#ifndef PROXYPOP3_POP3_SESSION_H
#define PROXYPOP3_POP3_SESSION_H

#include "queue.h"

// estados independientes de la maquina de estados general
enum pop3_session_state {
    POP3_AUTHORIZATION,
    POP3_TRANSACTION,
    POP3_UPDATE,
    POP3_DONE,
};

// representa una sesion pop3
struct pop3_session {
    char *user;
    char *password;

    enum pop3_session_state state;

    // historial de requests de la sesion
    struct pop3_request *first, *last;
    // todo podriamos hacer que la queue funcione de historial (para no tener una lista de requests aparte)
    // para eso cuando hacemos dequeue no tengo que perder la referencia al objeto

    // podria ser una variable global?? -> no porque me pueden cambiar el origin server?
    bool pipelining;
    // queue de requests, solo se usa si el server no soporta pipelining
    struct queue *request_queue;
};

void pop3_session_init(struct pop3_session *, bool);

#endif //PROXYPOP3_POP3_SESSION_H
