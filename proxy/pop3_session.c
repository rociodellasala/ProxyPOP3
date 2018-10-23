#include <string.h>

#include "include/pop3_session.h"

void pop3_session_init(struct pop3_session *s, bool pipelining) {
    memset(s, 0, sizeof(*s));

    s->pipelining = pipelining;

    if (s->pipelining == false) {
        s->request_queue = new_queue();
    }

    s->state = POP3_AUTHORIZATION;
}