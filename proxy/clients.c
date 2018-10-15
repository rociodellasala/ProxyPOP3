#include "include/clients.h"

clients_list new_list(){
    clients_list l = malloc(sizeof(struct clientList));
    l->first = l->last = NULL;
    l->size = 0;
    return l;
}

void add_client(clients_list l, client c){
    node_c * new_client = (node_c *) malloc(sizeof(node_c));
    new_client->c = c;

    if(l->first == NULL){
        l->first = new_client;
        l->last = new_client;
    } else {
        l->last->next = new_client;
        l->last = new_client;
    }

    l->last->next = NULL;
    l->size++;
}

/* Pegarle un ojo debe tener errores */
node_c * remove_client(clients_list l, client c){
    node_c * temp = l->first;

    if(l->first != NULL && l->first->c.client_fd == c.client_fd){
        l->first = l->first->next;
        return temp;
    }


    while(temp->next != NULL && temp->next->c.client_fd != c.client_fd) {
        temp = temp->next;
    }


    if(temp->next->c.client_fd == c.client_fd){
        l->size--;
        if(temp->next->c.client_fd == l->last->c.client_fd){
            l->last = temp;
        }
        temp->next = temp->next->next;
        return temp;
    }

    return NULL;
}

int is_empty(clients_list l){
    return (l-> first == NULL) ? 0 : 1;
}

unsigned int size(clients_list l){
    return l->size;
}

void print_current_clients(clients_list l){
    node_c * temp = l->first;

    while(temp != NULL){
        printf("Client: %d\n", temp->c.client_fd);
        temp = temp->next;
    }
}