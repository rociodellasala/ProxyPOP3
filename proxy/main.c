#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>

#include "include/selector.h"
#include "include/pop3.h"
#include "include/admin.h"
#include "include/input_parser.h"

/* Crea una conexión TCP o SCTP */
file_descriptor new_socket(int protocol, int port, struct sockaddr_in * address) {
    file_descriptor master_socket;
    int optval = 1;

    /* Crea el socket */
    master_socket = socket(AF_INET, SOCK_STREAM, protocol);

    if (master_socket < 0) {
        fprintf(stderr, "Unable to create %s socket", protocol == IPPROTO_TCP ? "TCP" : "SCTP");
        perror("");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0) {
        perror("'setsockopt()' failed");
        exit(EXIT_FAILURE);
    }

    /* Construye la dirección del socket */
    memset(address, 0, sizeof(*address));
    (*address).sin_family       = AF_INET;
    (*address).sin_addr.s_addr  = htonl(INADDR_ANY);
    (*address).sin_port         = htons((uint16_t) port);

    /* Enlaza el socket a la dirección especificada (puerto localhost)  */
    if (bind(master_socket, (struct sockaddr *) address, sizeof(*address)) < 0) {
        perror("Unable to bind socket");
        exit(EXIT_FAILURE);
    }

    /* Retorna el  file descriptor del socket */
    return master_socket;
}


int initialize_selector(file_descriptor mua_tcp_socket, file_descriptor admin_sctp_socket) {
    const char *err_msg;
    selector_status   ss      = SELECTOR_SUCCESS;
    fd_selector selector      = NULL;

    const struct selector_init conf = {
            .signal             = SIGALRM,
            .select_timeout     = {
                    .tv_sec  = 10,
                    .tv_nsec = 0,
            },
    };

    if (0 != selector_init(&conf)) {
        err_msg = "initializing selector_pop3";
        goto finally;
    }

    selector = selector_new(1024);

    if (selector == NULL) {
        err_msg = "unable to create selector";
        goto finally;
    }

    const struct fd_handler pop3_handler = {
            .handle_read       = &pop3_passive_accept,
            .handle_write      = NULL,
            .handle_close      = NULL, // nada que liberar
    };

    const struct fd_handler admin_handler = {
            .handle_read       = &admin_accept_connection,
            .handle_write      = NULL,
            .handle_close      = NULL,
    };

    /* Registro ambos fd en el selector para atender conexiones entrantes tanto de clientes como administradores */
    selector_status ss_pop3 = selector_register(selector, mua_tcp_socket, &pop3_handler, OP_READ, NULL);
    selector_status ss_manag = selector_register(selector, admin_sctp_socket, &admin_handler, OP_READ, NULL);

    if (ss_pop3 != SELECTOR_SUCCESS || ss_manag != SELECTOR_SUCCESS) {
        err_msg = "registering fd";
        goto finally;
    }

    for (;;) {
        err_msg  = NULL;
        ss  = selector_select(selector);
        if(ss != SELECTOR_SUCCESS) {
            err_msg = "serving";
            break;
        }
    }

    if (err_msg == NULL) {
        err_msg = "closing";
    }

    int ret = 0;

    finally:

    if (ss!= SELECTOR_SUCCESS) {
        fprintf(stderr, "%s: %s\n", (err_msg == NULL) ? "": err_msg,
                ss_pop3 == SELECTOR_IO ? strerror(errno) : selector_error(ss_pop3));
        ret = 2;
    } else if (err_msg) {
        perror(err_msg);
        ret = 1;
    }

    if (selector!= NULL) {
        selector_destroy(selector);
    }

    selector_close();

    pop3_pool_destroy();

    if (mua_tcp_socket >= 0) {
        close(mua_tcp_socket);
    }

    return ret;
}

int initialize_sockets(options opt) {
    struct sockaddr_in mua_address;
    struct sockaddr_in admin_address;

    file_descriptor mua_tcp_socket      = new_socket(IPPROTO_TCP, opt->port, &mua_address);                  // 1110
    file_descriptor admin_sctp_socket   = new_socket(IPPROTO_SCTP, opt->management_port, &admin_address);    // 9090

    /* Marca el socket como pasivo para que se quede escuchando conexiones entrantes y las acepte */
    if (listen(mua_tcp_socket, BACKLOG) < 0) {
        fprintf(stderr, "Unable to listen on port %d", opt->port);
        perror("");
        exit(EXIT_FAILURE);
    }

    printf("Listening on TCP port %d\n", opt->port);

    /* Marca el socket como pasivo para que se quede escuchando conexiones entrantes y las acepte */
    if (listen(admin_sctp_socket, BACKLOG) < 0) {
        fprintf(stderr, "Unable to listen on port %d", opt->management_port);
        perror("");
        exit(EXIT_FAILURE);
    }

    printf("Listening on SCTP port %d\n", opt->management_port);

    printf("Waiting for connections ...\n");

    return initialize_selector(mua_tcp_socket, admin_sctp_socket);
}

int main (int argc, char ** argv) {
    options opt;

    if (parse_input(argc, argv) < 0) {
        return -1;
    }

    opt = initialize_values();
    opt = set_options_values(argc, argv);
    printf("%s\n", opt->origin_server);

    return initialize_sockets(opt);
}