#ifndef PROXYPOP3_ADMINISTRATOR_H
#define PROXYPOP3_ADMINISTRATOR_H

#include "main.h"

#define BUFFER_SIZE 1024

/* Typedefs */
typedef enum {
   WAITING_EHLO, WAITING_USER, WAITING_PASS
} status;

typedef struct {
    file_descriptor admin_fd;
    status current_status;
} admin;

/* Functions */
admin accept_admin_connection(proxy_pop3 *);
void administrator_read(admin *);
status parse_ehlo(char *, admin);
status parse_admin_command(admin, char **);

#endif //PROXYPOP3_ADMINISTRATOR_H
