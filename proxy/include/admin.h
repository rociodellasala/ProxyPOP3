#ifndef PROXYPOP3_ADMIN_H
#define PROXYPOP3_ADMIN_H

#include <netinet/in.h>
#include "selector.h"
#include "buffer.h"
#include "request_admin.h"
#include "response_admin.h"

#define BUFFER_SIZE 1024

typedef enum parse_status {
    ST_AUTH     = 1,
    ST_TRANS = 2
} parse_status;

struct admin {
    struct sockaddr_storage       client_addr;
    socklen_t                     client_addr_len;
    int                           client_fd;

    buffer                        buffer_write, buffer_read;
    uint8_t                       raw_buffer_write[BUFFER_SIZE], raw_buffer_read[BUFFER_SIZE];

    parse_status                  status;
    int                           argc;
};

void admin_accept_connection(struct selector_key *);
int parse_commands(struct admin *);


#endif  //PROXYPOP3_ADMIN_H
