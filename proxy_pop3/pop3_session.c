#include <string.h>
#include <stdlib.h>
#include "include/pop3_session.h"

void pop3_session_init(struct pop3_session * session, bool pipelining) {
    memset(session, 0, sizeof(*session));
    session->pipelining = pipelining;
    session->state = POP3_AUTHORIZATION;
    session->request_queue = new_queue();
}

void pop3_session_close(struct pop3_session * session) {
    if (session->request_queue != NULL) {
        destroy_queue(session->request_queue);
    }

    free(session->user_name);
    session->state = POP3_DONE;
}