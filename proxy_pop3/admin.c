#include <sys/socket.h>
#include <stdlib.h>
#include <memory.h>

#include "include/input_parser.h"
#include "include/admin.h"
#include "include/admin_actions.h"
#include "include/admin_parser.h"
#include "include/metrics.h"
#include "include/logs.h"

#define ATTACHMENT(key) ((struct admin *)(key)->data)
#define N(x) (sizeof(x)/sizeof((x)[0]))

options parameters;

static const struct fd_handler admin_handler = {
        .handle_read   = admin_read,
        .handle_write  = admin_write,
        .handle_close  = admin_close,
};

struct admin * admin_new(const int admin_fd, struct sockaddr_storage * client_addr) {
    struct admin * new_admin;

    new_admin = malloc(sizeof(*new_admin));

    if (new_admin == NULL){
        return new_admin;
    }

    new_admin->fd                   = admin_fd;
    new_admin->a_status             = ST_EHLO;
    new_admin->req_status           = REQ_PARSE_OK;
    new_admin->resp_status          = RESP_PARSE_OK;
    new_admin->resp_length          = 0;
    new_admin->quit                 = 0;
    new_admin->admin_sock_addr_addr = (const struct sockaddr *) client_addr;

    return new_admin;
}


void admin_accept_connection(struct selector_key * key) {
    struct sockaddr_storage     client_addr;
    socklen_t                   client_addr_len   = sizeof(client_addr);
    struct admin *              new_admin         = NULL;

    const file_descriptor client = accept(key->fd, (struct sockaddr*) &client_addr, &client_addr_len);
    metric_add_admin_connected();

    if (client == -1) {
        log_connection(false, (const struct sockaddr *) &client_addr, "ADMIN connection failed");
        goto fail;
    } else {
        log_connection(true, (const struct sockaddr *) &client_addr, "ADMIN connection success");
    }

    if (selector_fd_set_nio(client) == -1) {
        goto fail;
    }

    new_admin = admin_new(client, &client_addr);

    if (new_admin == NULL) {
        goto fail;
    }

    memcpy((void *) &new_admin->admin_addr, &client_addr, client_addr_len);

    /* Empezamos por mandar el mensaje de bienvenida al cliente administrador */
    if (SELECTOR_SUCCESS != selector_register(key->s, client, &admin_handler, OP_WRITE, new_admin)) {
        goto fail;
    }

    return ;

    fail:

    if (client != -1) {
        close(client);
    }

    free(new_admin);
}

void admin_read(struct selector_key * key) {
    struct admin * admin = ATTACHMENT(key);
    
    parse_admin_request(admin);
    
    if (selector_set_interest(key->s, key->fd, OP_WRITE) != SELECTOR_SUCCESS) {
        selector_unregister_fd(key->s, admin->fd);
    }
}

void reset_admin_status(struct admin * admin) {
    admin->current_request  = NULL;
    admin->resp_length      = 0;
    admin->resp_data        = NULL;
    admin->req_status       = REQ_PARSE_OK;
    admin->resp_status      = RESP_PARSE_OK;
}

void admin_write(struct selector_key * key) {
    int             resp    = -1;
    struct admin *  admin   = ATTACHMENT(key);

    if (admin->quit == 0) {
        while(resp < 0) {
            resp = parse_admin_response(admin);
        }
    } else {
        quit_admin(admin);

        if (admin->current_request->length > 0) {
            free(admin->current_request->data);
        }

        //free(admin->current_request);
        metric_remove_admin_connected();
        selector_unregister_fd(key->s, admin->fd);
        return;
    }

    if (selector_set_interest(key->s, key->fd, OP_READ) != SELECTOR_SUCCESS) {
        selector_unregister_fd(key->s, admin->fd);
    }

    reset_admin_status(admin);
}

void admin_close(struct selector_key * key) {
    struct admin * admin = ATTACHMENT(key);
    log_connection(true, admin->admin_sock_addr_addr, "ADMIN disconnected");
    free(admin);
}
