#ifndef PROXYPOP3_ADMINISTRATOR_H
#define PROXYPOP3_ADMINISTRATOR_H

#include "main.h"
#include "request.h"
#include "response.h"

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
void administrator_read(admin *, options);
void admin_read_handler(admin *, char *, options *);

ssize_t send_response(file_descriptor, response *);
void send_response_without_data( file_descriptor, int);
void send_response_with_data(char *, file_descriptor, int);

int check_password(request *, int *);
void forbid_mime(request *, int, char *);
void allow_mime(request *, int, char *);

#endif //PROXYPOP3_ADMINISTRATOR_H
