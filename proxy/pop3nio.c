/**
 * pop3nio.c  - controla el flujo de un proxy POP3 (sockets no bloqueantes)
 */
#include <stdio.h>
#include <stdlib.h>  // malloc
#include <string.h>  // memset
#include <assert.h>  // assert
#include <errno.h>
#include <time.h>
#include <unistd.h>  // close

#include <arpa/inet.h>
#include <pthread.h>

#include "include/buffer.h"

#include "include/stm.h"
#include "include/pop3nio.h"
#include "include/input_parser.h"
#include "include/utils.h"
#include "include/metrics.h"

#define N(x) (sizeof(x)/sizeof((x)[0]))

/* Maquina de estados general */
enum pop3_state {
    /**
     * Estado inicial donde se resuelve el nombre del origin server
     * a traves de una resolución DNS
     *
     * Transiciones:
     *         - CONNECTING_TO_OS   una vez realizada la resolucion DNS
     */
     ORIGIN_SERVER_RESOLUTION,
     CONNECTING_TO_OS,
     READING_WELCOME_FROM_OS,
     CAPA,
     // estados terminales
     DONE,
     ERROR,
};

struct welcome_st {
    /** buffer utilizado para I/O */
    buffer * write_buffer;
};


/*
 * Si bien cada estado tiene su propio struct que le da un alcance
 * acotado, disponemos de la siguiente estructura para hacer una única
 * alocación cuando recibimos la conexión.
 *
 * Se utiliza un contador de referencias (references) para saber cuando debemos
 * liberarlo finalmente, y un pool para reusar alocaciones previas.
 */
struct client_pop3 {
    /** información del cliente */
    struct sockaddr_storage       client_addr;
    socklen_t                     client_addr_len;
    int                           client_fd;

    /** resolución de la dirección del origin server */
    struct addrinfo *             origin_resolution;
    /** intento actual de la dirección del origin server */
    struct addrinfo *             origin_resolution_current;

    /** información del origin server */
    struct sockaddr_storage       origin_addr;
    socklen_t                     origin_addr_len;
    int                           origin_domain;
    int                           origin_fd;


    /** maquinas de estados */
    struct state_machine          stm;

    /** estados para el client_fd */
    union {

    } client;

    /** estados para el origin_fd */
    union {
        struct welcome_st       welcome;
    } origin;

    /** buffers para ser usados read_buffer, write_buffer.*/
    uint8_t raw_buff_a[2048], raw_buff_b[2048];
    buffer read_buffer, write_buffer;

    /** cantidad de referencias a este objeto. si es uno se debe destruir */
    unsigned references;

    /** siguiente en el pool */
    struct client_pop3 * next;
};

/**
 * Pool de `struct pop3', para ser reusados.
 *
 * Como tenemos un unico hilo que emite eventos no necesitamos barreras de
 * contención.
 */

static const unsigned       max_pool  = 50; // tamaño máximo
static unsigned             pool_size = 0;  // tamaño actual
static struct client_pop3 * pool      = 0;  // pool propiamente dicho

static const struct state_definition * pop3_describe_states(void);

/* Crea un nuevo `struct pop3' */
static struct client_pop3 * pop3_new(int client_fd) {
    struct client_pop3 * ret;

    if (pool == NULL) {
        ret = malloc(sizeof(*ret));
    } else {
        ret       = pool;
        pool      = pool->next;
        ret->next = 0;
    }

    if (ret == NULL) {
        goto finally;
    }

    memset(ret, 0x00, sizeof(*ret));

    ret->origin_fd          = -1;
    ret->client_fd          = client_fd;
    ret->client_addr_len    = sizeof(ret->client_addr);

    ret->stm    .initial    = ORIGIN_SERVER_RESOLUTION;
    ret->stm    .max_state  = ERROR;
    ret->stm    .states     = pop3_describe_states();

    stm_init(&ret->stm);

    buffer_init(&ret->read_buffer,  N(ret->raw_buff_a), ret->raw_buff_a);
    buffer_init(&ret->write_buffer, N(ret->raw_buff_b), ret->raw_buff_b);

    ret->references = 1;

    finally:
    return ret;
}

/* Realmente destruye */
static void pop3_destroy_(struct client_pop3 * s) {
    if (s->origin_resolution != NULL) {
        freeaddrinfo(s->origin_resolution);
        s->origin_resolution = 0;
    }

    free(s);
}

/**
 * Destruye un `struct pop3', tiene en cuenta las referencias
 * y el pool de objetos.
 */
static void pop3_destroy(struct client_pop3 * s) {
    if (s == NULL) {
        // nada para hacer
    } else if (s->references == 1) {
        if (s != NULL) {
            if (pool_size < max_pool) {
                s->next = pool;
                pool    = s;
                pool_size++;
            } else {
                pop3_destroy_(s);
            }
        }
    } else {
        s->references -= 1;
    }
}

void pop3_pool_destroy(void) {
    struct client_pop3 * next, * s;

    for (s = pool; s != NULL; s = next) {
        next = s->next;
        free(s);
    }
}

/* Obtiene el struct (pop3 *) desde la llave de selección  */
#define ATTACHMENT(key) ((struct client_pop3 *)(key)->data)

/*
 * Declaración forward de los handlers de selección de una conexión
 * establecida entre un cliente y el proxy.
 */
static void pop3_read(struct selector_key * key);
static void pop3_write(struct selector_key * key);
static void pop3_block(struct selector_key * key);
static void pop3_close(struct selector_key * key);

static const struct fd_handler pop3_handler = {
        .handle_read   = pop3_read,
        .handle_write  = pop3_write,
        .handle_close  = pop3_close,
        .handle_block  = pop3_block,
};

/* Intenta aceptar la nueva conexión entrante */
void pop3_accept_connection(struct selector_key *key) {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len   = sizeof(client_addr);
    struct client_pop3 * state  = NULL;

    const file_descriptor client = accept(key->fd, (struct sockaddr*) &client_addr, &client_addr_len);

    metric_add_new_connection();

    if (client == -1) {
        goto fail;
    }

    if (selector_fd_set_nio(client) == -1) {
        goto fail;
    }

    print_connection_status("[CLIENT]: Connection established", client_addr, client);
    state = pop3_new(client);

    if (state == NULL) {
        // sin un estado, nos es imposible manejaro.
        // tal vez deberiamos apagar accept() hasta que detectemos
        // que se liberó alguna conexión.
        goto fail;
    }

    memcpy(&state->client_addr, &client_addr, client_addr_len);
    state->client_addr_len = client_addr_len;

    if (SELECTOR_SUCCESS != selector_register(key->s, client, &pop3_handler,
                                             OP_WRITE, state)) {
        goto fail;
    }

    return ;

    fail:
    if (client != -1) {
        close(client);
    }

    pop3_destroy(state);
}



////////////////////////////////////////////////////////////////////////////////
// ORIGIN SERVER RESOLUTION
////////////////////////////////////////////////////////////////////////////////

static void * os_resolv_blocking(void * data);

static unsigned origin_server_connect(struct selector_key * key);

/*
 * Tenemos que resolver el nombre (operación bloqueante), por ende disparamos
 * la resolución en un thread que luego notificará al selector que ha terminado.
 */
unsigned origin_server_resolution(struct selector_key * key) {
    pthread_t tid;
    /* Creamos una key para pasarle como argumento al thread */
    struct selector_key * k = malloc(sizeof(*key));

    if (k == NULL) {
        return ERROR;
    } else {
        memcpy(k, key, sizeof(*k));
        if (pthread_create(&tid, 0, os_resolv_blocking, k) == -1) {
            return ERROR;
        } else {
            selector_set_interest_key(key, OP_NOOP);
        }
    }

    return ORIGIN_SERVER_RESOLUTION;
}

/**
 * Realiza la resolución de DNS bloqueante.
 *
 * Una vez resuelto notifica al selector para que el evento esté
 * disponible en la próxima iteración.
 */
static void * os_resolv_blocking(void * k) {
    struct selector_key * key = (struct selector_key *) k;
    struct client_pop3 *  s   = ATTACHMENT(key);
    char buff[7];

    pthread_detach(pthread_self());
    s->origin_resolution = 0;

    struct addrinfo hints = {
            .ai_family    = AF_UNSPEC,    /* Allow IPv4 or IPv6 */
            .ai_socktype  = SOCK_STREAM,  /* Datagram socket */
            .ai_flags     = AI_PASSIVE,   /* For wildcard IP address */
            .ai_protocol  = 0,            /* Any protocol */
            .ai_canonname = NULL,
            .ai_addr      = NULL,
            .ai_next      = NULL,
    };


    snprintf(buff, sizeof(buff), "%hu", parameters->origin_port);

    if (0 != getaddrinfo(parameters->origin_server, buff, &hints, &s->origin_resolution)){
        fprintf(stderr,"Domain name resolution error\n");
    }

    selector_notify_block(key->s, key->fd);

    free(k);

    return 0;
}

/* Procesa el resultado de la resolución de nombres */
static unsigned origin_server_resolution_done(struct selector_key * key) {
    struct client_pop3 * s =  ATTACHMENT(key);

    if(s->origin_resolution == 0) {
        // todo
        return ERROR;
    } else {
        s->origin_domain   = s->origin_resolution->ai_family;
        s->origin_addr_len = s->origin_resolution->ai_addrlen;
        memcpy(&s->origin_addr,
               s->origin_resolution->ai_addr,
               s->origin_resolution->ai_addrlen);
        freeaddrinfo(s->origin_resolution);
        s->origin_resolution = 0;
    }

    return origin_server_connect(key);
}

/* Intenta establecer una conexión con el origin server */
static unsigned origin_server_connect(struct selector_key * key) {
    bool error                  = false;
    file_descriptor oserver_fd = socket(ATTACHMENT(key)->origin_domain, SOCK_STREAM, IPPROTO_TCP);

    if (oserver_fd == -1) {
        error = true;
        goto finally;
    }

    if (selector_fd_set_nio(oserver_fd) == -1) {
        goto finally;
    }

    if (connect(oserver_fd, (const struct sockaddr *)&ATTACHMENT(key)->origin_addr,
                      ATTACHMENT(key)->origin_addr_len) == -1) {
        if (errno == EINPROGRESS) {
            // es esperable,  tenemos que esperar a la conexión
            // dejamos de de pollear el socket del cliente
            selector_status st = selector_set_interest_key(key, OP_NOOP);
            if (SELECTOR_SUCCESS != st) {
                error = true;
                goto finally;
            }

            // esperamos la conexion en el nuevo socket
            st = selector_register(key->s, oserver_fd, &pop3_handler, OP_WRITE, key->data);

            if (SELECTOR_SUCCESS != st) {
                error = true;
                goto finally;
            }

            ATTACHMENT(key)->references += 1;
        } else {
            error = true;
            goto finally;
        }
    } else {
        // estamos conectados sin esperar... no parece posible
        abort();
    }

    /* Empezamos por mandar el mensaje de bienvenida al cliente */
    if (SELECTOR_SUCCESS != selector_set_interest_key(key, OP_WRITE)) {
        return ERROR;
    }

    return CONNECTING_TO_OS;

    finally:
    if (error) {
        if (oserver_fd != -1) {
            close(oserver_fd);
        }
    }

    return ERROR;
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// CONNECTING TO OS
////////////////////////////////////////////////////////////////////////////////

void
connecting_init(const unsigned state, struct selector_key * key) {
    // nada por hacer
}

void send_error_(int fd, const char * error) {
    send(fd, error, strlen(error), 0);
}

unsigned connecting(struct selector_key * key) {
    int error;
    socklen_t len = sizeof(error);
    struct client_pop3 *d = ATTACHMENT(key);

    d->origin_fd = key->fd;


    if (getsockopt(key->fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        send_error_(d->client_fd, "-ERR Connection refused.\r\n");
        fprintf(stderr, "Connection to origin server failed\n");
        selector_set_interest_key(key, OP_NOOP);
        return ERROR;
    } else {
        if(error == 0) {
            d->origin_fd = key->fd;
        } else {
            send_error_(d->client_fd, "-ERR Connection refused.\r\n");
            fprintf(stderr, "Connection to origin server failed\n");
            selector_set_interest_key(key, OP_NOOP);
            return ERROR;
        }
    }

    // iniciamos la sesion pop3 sin pipelining del lado del server
    //pop3_session_init(&ATTACHMENT(key)->session, false);

    selector_status ss = SELECTOR_SUCCESS;

    ss |= selector_set_interest_key(key, OP_READ);
    ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_NOOP);

    return SELECTOR_SUCCESS == ss ? READING_WELCOME_FROM_OS : ERROR;
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// READING WELCOME FROM OS
////////////////////////////////////////////////////////////////////////////////

/* Inicializa las variables del estado READING WELCOME FROM OS */
static void welcome_init(const unsigned state, struct selector_key * key) {
    struct welcome_st * d = &ATTACHMENT(key)->origin.welcome;
    d->write_buffer = &(ATTACHMENT(key)->write_buffer);
}

/** Lee todos los bytes del mensaje de tipo `hello' de server_fd */
static unsigned welcome_read(struct selector_key * key) {
    struct welcome_st *   d   = &ATTACHMENT(key)->origin.welcome;
    enum pop3_state     ret = READING_WELCOME_FROM_OS;
    uint8_t * ptr;
    size_t  count;
    ssize_t  n;

    ptr = buffer_write_ptr(d->write_buffer, &count);
    const char * msg = "Proxy server POP3\r\n";
    n = strlen(msg);
    strcpy((char *) ptr, msg);
    memccpy(ptr, msg, 0, count);
    buffer_write_adv(d->write_buffer, n);
    
    ptr = buffer_write_ptr(d->write_buffer, &count);
    n = recv(key->fd, ptr, count, 0);

    if (n > 0) {
        buffer_write_adv(d->write_buffer, 0);

        selector_status ss = SELECTOR_SUCCESS;
        ss |= selector_set_interest_key(key, OP_NOOP);
        ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_WRITE);
        if (ss != SELECTOR_SUCCESS) {
            ret = ERROR;
        }
    } else {
        ret = ERROR;
    }

    return ret;
}

/** Escribe todos los bytes del mensaje `hello' en client_fd */
static unsigned welcome_write(struct selector_key *key) {
    struct welcome_st * d = &ATTACHMENT(key)->origin.welcome;

    unsigned  ret      = READING_WELCOME_FROM_OS;
    uint8_t *ptr;
    size_t  count;
    ssize_t  n;

    ptr = buffer_read_ptr(d->write_buffer, &count);
    n = send(key->fd, ptr, count, MSG_NOSIGNAL);

    if(n == -1) {
        ret = ERROR;
    } else {
        buffer_read_adv(d->write_buffer, n);
        if(!buffer_can_read(d->write_buffer)) {
            selector_status ss = SELECTOR_SUCCESS;
            ss |= selector_set_interest_key(key, OP_NOOP);
            ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_READ);
            ret = SELECTOR_SUCCESS == ss ? CAPA : ERROR;

            if (ret == CAPA) {
                char * msg = "CAPA\r\n";
                send(ATTACHMENT(key)->origin_fd, msg, strlen(msg), 0);
            }
        }
    }

    return ret;
}

static void
welcome_close(const unsigned state, struct selector_key * key) {
    //nada por hacer
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// CAPA
////////////////////////////////////////////////////////////////////////////////

void
capa_init(const unsigned state, struct selector_key *key) {
    printf("EN CAPA\n");
}

////////////////////////////////////////////////////////////////////////////////

/* Definición de handlers para cada estado */
static const struct state_definition client_statbl[] = {
        {
                .state            = ORIGIN_SERVER_RESOLUTION,
                .on_write_ready   = origin_server_resolution,
                .on_block_ready   = origin_server_resolution_done,
        },{
                .state            = CONNECTING_TO_OS,
                .on_arrival       = connecting_init,
                .on_write_ready   = connecting,
        },{
                .state            = READING_WELCOME_FROM_OS,
                .on_arrival       = welcome_init,
                .on_read_ready    = welcome_read,
                .on_write_ready   = welcome_write,
                .on_departure     = welcome_close,
        },{
                .state            = CAPA,
                .on_arrival       = capa_init,
                //.on_read_ready  = capa_read,
        },{
                .state            = DONE,

        },{
                .state            = ERROR,
        }
};

static const struct state_definition * pop3_describe_states(void) {
    return client_statbl;
}

///////////////////////////////////////////////////////////////////////////////
// Handlers top level de la conexión pasiva.
// son los que emiten los eventos a la maquina de estados.
static void pop3_done(struct selector_key * key);

static void pop3_read(struct selector_key * key) {
    struct state_machine * stm      = &ATTACHMENT(key)->stm;
    const enum pop3_state st        = (enum pop3_state) stm_handler_read(stm, key);

    if (ERROR == st || DONE == st) {
        pop3_done(key);
    }
}

static void pop3_write(struct selector_key * key) {
    struct state_machine * stm   = &ATTACHMENT(key)->stm;
    const enum pop3_state st    = (enum pop3_state)stm_handler_write(stm, key);

    if (ERROR == st || DONE == st) {
        pop3_done(key);
    }
}

static void pop3_block(struct selector_key * key) {
    struct state_machine *stm   = &ATTACHMENT(key)->stm;
    const enum pop3_state st    = (enum pop3_state) stm_handler_block(stm, key);

    if (ERROR == st || DONE == st) {
        pop3_done(key);
    }
}

static void pop3_close(struct selector_key * key) {
    pop3_destroy(ATTACHMENT(key));
}

static void pop3_done(struct selector_key * key) {
    unsigned int i;

    const int fds[] = {
            ATTACHMENT(key)->client_fd,
            ATTACHMENT(key)->origin_fd,
    };

    for (i = 0; i < N(fds); i++) {
        if(fds[i] != -1) {
            if(SELECTOR_SUCCESS != selector_unregister_fd(key->s, fds[i])) {
                abort();
            }
            close(fds[i]);
        }
    }
}
