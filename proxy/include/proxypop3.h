#ifndef PROXYPOP3_PROXYPOP3_H
#define PROXYPOP3_PROXYPOP3_H

#include "input_parser.h"
#include "handler.h"
#include "clients.h"
#include "administrator.h"

#define BUFFER_SIZE 1024
#define MAX_CLIENTS FD_SETSIZE

/* Typedefs */
typedef int file_descriptor;

/* Functions */
void initialize_proxy(options, file_descriptor, struct sockaddr_in, file_descriptor, struct sockaddr_in);
clients_list accept_user_connection(proxy_pop3 *, clients_list);
void remove_user_connection(struct sockaddr_in address, client cl, clients_list clients);
file_descriptor add_client_to_set(fd_set * readfds, clients_list clients, file_descriptor max_fd);
file_descriptor add_admin_to_set(fd_set * readfds, admin admin, file_descriptor max_fd);

#endif //PROXYPOP3_PROXYPOP3_H
