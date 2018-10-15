#ifndef PROXYPOP3_ADMINISTRATOR_H
#define PROXYPOP3_ADMINISTRATOR_H

#include "main.h"

/* Typedefs */
typedef enum {
   WAITING_USER, WAITING_PASSWORD
} status;

typedef struct {
    file_descriptor admin_fd;
    status current_status;
} admin;

/* Functions */
admin accept_admin_connection(proxy_pop3 * proxy);

#endif //PROXYPOP3_ADMINISTRATOR_H
