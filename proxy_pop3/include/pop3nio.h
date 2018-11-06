#ifndef PROXYPOP3_POP3_H
#define PROXYPOP3_POP3_H

#include "selector.h"
#include "utils.h"

#include "client_parser_response.h"
#include "client_parser_request.h"
#include "pop3_session.h"
#include "stm.h"
#include "pop3_handler.h"
#include "external_transformation.h"

/* Obtiene el struct (pop3 *) desde la llave de selección  */
#define ATTACHMENT(key) ((struct pop3 *)(key)->data)
#define N(x) (sizeof(x)/sizeof((x)[0]))

/*
 * Maquina de estados general
 */
enum pop3_state {
    ERROR = -1,
    ORIGIN_SERVER_RESOLUTION,
    CONNECTING_TO_OS,
    WELCOME_READ,
    WELCOME_WRITE,
    CAPA,
    REQUEST,
    RESPONSE,
    EXTERNAL_TRANSFORMATION,
    DONE,
};

/*
 * Declaración de los handlers de selección de una conexión
 * establecida entre un cliente y el pop3filter.
 */
static const struct fd_handler pop3_handler = {
        .handle_read   = pop3_read,
        .handle_write  = pop3_write,
        .handle_close  = pop3_close,
        .handle_block  = pop3_block,
};


/*
 * Definición de variables para cada estado
 */
struct welcome_st {
    buffer *                    buffer;
    bool                        msg_not_finished;
};

struct request_st {
    buffer *                    read_buffer;
    buffer *                    write_buffer;

    struct pop3_request         request;
    struct request_parser       request_parser;

};

/* Usado por RESPONSE */
struct response_st {
    buffer *                    read_buffer;
    buffer *                    write_buffer;

    struct pop3_request *       request;

    struct response_parser      response_parser;
};

/* TODO: ver de achicarlo */
#define BUFFER_SIZE 45

/*
 * Si bien cada estado tiene su propio struct que le da un alcance
 * acotado, disponemos de la siguiente estructura para hacer una única
 * alocación cuando recibimos la conexión.
 *
 * Se utiliza un contador de referencias (references) para saber cuando debemos
 * liberarlo finalmente, y un pool para reusar alocaciones previas.
 */
struct pop3 {
    struct sockaddr_storage         client_addr;
    file_descriptor                 client_fd;

    struct addrinfo *               origin_resolution;

    struct sockaddr_storage         origin_addr;
    socklen_t                       origin_addr_len;
    int                             origin_domain;
    file_descriptor                 origin_fd;

    struct pop3_session             session;

    struct state_machine            stm;

    union {
        struct request_st           request;
    } client;

    union {
        struct welcome_st           welcome;
        struct response_st          response;
    } orig;


    uint8_t                         raw_buff_a[BUFFER_SIZE];
    uint8_t                         raw_buff_b[BUFFER_SIZE];
    buffer                          read_buffer;
    buffer                          write_buffer;

    uint8_t                         raw_super_buffer[BUFFER_SIZE];
    buffer                          super_buffer;

    uint8_t                         raw_extern_read_buffer[BUFFER_SIZE];
    buffer                          extern_read_buffer;

    int                             extern_read_fd;
    int                             extern_write_fd;
    struct external_transformation  et;

    unsigned                        references;

    struct pop3 *                   next;
};

/*
 * Handler del socket pasivo que atiende conexiones pop3
 */
void pop3_accept_connection(struct selector_key *);

/*
 * Libera pools internos
 */
void pop3_pool_destroy(void);

/*
 * Destruye un `struct pop3', tiene en cuenta las referencias
 * y el pool de objetos.
 */
void pop3_destroy(struct pop3 * pop3);

#endif //PROXYPOP3_POP3_H
