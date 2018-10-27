#include <sys/socket.h>
#include <unistd.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>

#include "include/selector.h"
#include "include/input_parser.h"
#include "include/utils.h"
#include "include/admin.h"
#include "include/admin_request.h"
#include "include/deserializer.h"
#include "include/admin_actions.h"
#include "include/admin_parser.h"

#define ATTACHMENT(key) ((struct admin *)(key)->data)
#define N(x) (sizeof(x)/sizeof((x)[0]))

options parameters;

static const struct fd_handler admin_handler = {
        .handle_read   = admin_read,
        .handle_write  = admin_write,
        .handle_close  = admin_close,
};

struct admin * admin_new(const int admin_fd) {
    struct admin * ret;
    ret = malloc(sizeof(*ret));

    if (ret == NULL){
        return ret;
    }

    ret->fd             = admin_fd;
    ret->a_status       = ST_EHLO;
    ret->req_status     = REQ_PARSE_OK;
    ret->resp_status    = RESP_PARSE_OK;
    ret->resp_length    = 0;
    ret->quit           = 0;

    return ret;
}


void admin_accept_connection(struct selector_key * key) {
    struct sockaddr_storage       client_addr;
    socklen_t                     client_addr_len = sizeof(client_addr);
    struct admin *                state           = NULL;

    const int client = accept(key->fd, (struct sockaddr*) &client_addr, &client_addr_len);

    if(client == -1) {
        goto fail;
    }

    if(selector_fd_set_nio(client) == -1) {
        goto fail;
    }

    print_connection_status("[ADMIN]: Connection established", client_addr);
    printf("[ADMIN]: File descriptor: %d\n", client);
    state = admin_new(client);

    if(state == NULL) {
        goto fail;
    }

    memcpy(&state->admin_addr, &client_addr, client_addr_len);

    /* Empezamos por mandar el mensaje de bienvenida al cliente administrador */
    if(SELECTOR_SUCCESS != selector_register(key->s, client, &admin_handler, OP_WRITE, state)) {
        goto fail;
    }

    return ;

    fail:
    if(client != -1) {
        close(client);
    }

    free(state);
}

void admin_read(struct selector_key * key){
    struct admin * admin = ATTACHMENT(key);
    
    parse_admin_request(admin);
    
    
    if(selector_set_interest(key->s, key->fd, OP_WRITE) != SELECTOR_SUCCESS){
        selector_unregister_fd(key->s, admin->fd);
    };
}

void reset_admin_status(struct admin * admin) {
    admin->current_request  = NULL;
    admin->resp_length      = 0;
    admin->resp_data        = NULL;
    admin->req_status       = REQ_PARSE_OK;
    admin->resp_status      = RESP_PARSE_OK;
}

void admin_write(struct selector_key * key) {
    struct admin * admin = ATTACHMENT(key);
    int resp = -1;

    if (admin->quit == 0) {
        while(resp < 0){ //TODO: chequear
            resp = parse_admin_response(admin);
        }
    } else {
        quit(admin);
        selector_unregister_fd(key->s, admin->fd);
        return;
    }

    if(selector_set_interest(key->s, key->fd, OP_READ) != SELECTOR_SUCCESS){
        selector_unregister_fd(key->s, admin->fd);
    };

    reset_admin_status(admin);
}

void admin_close(struct selector_key * key) {
    struct admin * admin = ATTACHMENT(key);
    print_connection_status("[ADMIN]: Connection disconnected", admin->admin_addr);
    free(admin);
}
