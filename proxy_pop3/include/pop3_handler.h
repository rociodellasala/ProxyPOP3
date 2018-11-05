#ifndef PROXYPOP3_POP3_HANDLER_H
#define PROXYPOP3_POP3_HANDLER_H

#include "selector.h"

void pop3_read(struct selector_key * key);
void pop3_write(struct selector_key * key);
void pop3_block(struct selector_key * key);
void pop3_close(struct selector_key * key);

#endif //PROXYPOP3_POP3_HANDLER_H
