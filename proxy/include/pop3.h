#ifndef PROXYPOP3_POP3_H
#define PROXYPOP3_POP3_H

#include <netdb.h>
#include "selector.h"

/* Handler del socket pasivo que atiende conexiones pop3 */
void pop3_passive_accept(struct selector_key *);

/* Libera pools internos */
void pop3_pool_destroy(void);

#endif //PROXYPOP3_POP3_H
