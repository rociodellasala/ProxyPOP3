#ifndef PROXYPOP3_POP3_SESSION_H
#define PROXYPOP3_POP3_SESSION_H

#include <stdbool.h>

#include "queue.h"

/* Estados POP3 */
enum pop3_session_state {
    POP3_AUTHORIZATION,
    POP3_TRANSACTION,
    POP3_UPDATE,
    POP3_DONE,
};

/* Representa una sesion pop3 */
struct pop3_session {
    // long maxima: 40 bytes segun rfc de pop3
    char * user;
    char * password;

    enum pop3_session_state state;
    unsigned concurrent_invalid_commands;

    bool pipelining;

    struct queue * request_queue;
};

void pop3_session_init(struct pop3_session *s, bool pipelining);

#endif //PROXYPOP3_POP3_SESSION_H

