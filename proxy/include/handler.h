#ifndef PROXYPOP3_HANDLER_H
#define PROXYPOP3_HANDLER_H

#define BUFFER_SIZE 1024

void handle_connections(proxy_pop3 proxy, struct timespec timeout);

#endif //PROXYPOP3_HANDLER_H
