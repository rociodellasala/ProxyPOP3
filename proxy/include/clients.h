#ifndef PROXYPOP3_CLIENTS_H
#define PROXYPOP3_CLIENTS_H

#include "main.h"

/* Typedefs */
typedef struct {
    file_descriptor server_fd;
    file_descriptor client_fd;
} client;

typedef struct clientNode node_c;

struct clientNode{
    client c;
    node_c * next;
};

typedef struct clientList * clients_list;

struct clientList {
    node_c * first;
    node_c * last;
    int size;
};

/* Functions */
clients_list new_list();
void add_client(clients_list, client);
node_c * remove_client(clients_list, client);
int is_empty(clients_list);
unsigned int size(clients_list);
void print_current_clients(clients_list);

#endif //PROXYPOP3_CLIENTS_H
