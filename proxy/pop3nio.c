/**
 * pop3.c  - controla el flujo de un proxy POP3 (sockets no bloqueantes)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <pthread.h>

#include "include/pop3_session.h"
#include "include/request_parser.h"
#include "include/buffer.h"

#include "include/stm.h"
#include "include/pop3nio.h"
#include "include/input_parser.h"
#include "include/utils.h"
#include "include/metrics.h"

#define N(x) (sizeof(x)/sizeof((x)[0]))

/* Maquina de estados general */
enum pop3_state {
            ORIGIN_SERVER_RESOLUTION,
            CONNECTING_TO_OS,
            WELCOME_READ,
            WELCOME_WRITE,
            //CAPA,
            REQUEST_READ,
            REQUEST_WRITE,
            RESPONSE_READ,
            RESPONSE_WRITE,
            DONE,
            ERROR,
};

////////////////////////////////////////////////////////////////////
// Definición de variables para cada estado

/* Usado por HELLO_READ, HELLO_WRITE */
struct welcome_st {
    /** buffer utilizado para I/O */
    buffer * wb;
};

/* Usado por REQUEST_READ y REQUEST_WRITE */
struct request_st {
    /** buffer utilizado para I/O */
    buffer *                     wb;
    buffer *                     rb;

    /** parser */
    struct pop3_request         request;
    struct request_parser       parser;
};

/* Usado por RESPONSE_READ y RESPONSE_WRITE */
struct response_st {
    /** buffer utilizado para I/O */
    buffer *                     wb;
    buffer *                     rb;

    /** parser */
    struct pop3_request *       request;
    //struct response_parser      parser;
};


/*
 * Si bien cada estado tiene su propio struct que le da un alcance
 * acotado, disponemos de la siguiente estructura para hacer una única
 * alocación cuando recibimos la conexión.
 *
 * Se utiliza un contador de referencias (references) para saber cuando debemos
 * liberarlo finalmente, y un pool para reusar alocaciones previas.
 */
struct pop3 {
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

    struct pop3_session           session;

    /** maquinas de estados */
    struct state_machine          stm;

    /** estados para el client_fd */
    union {
        struct request_st         request;
    } client;
    /** estados para el origin_fd */
    union {
        struct welcome_st         welcome;
    } orig;

    /** buffers para ser usados read_buffer, write_buffer.*/
    uint8_t raw_buff_a[2048], raw_buff_b[2048];
    buffer read_buffer, write_buffer;

    /** cantidad de referencias a este objeto. si es uno se debe destruir */
    unsigned references;

    /** siguiente en el pool */
    struct pop3 * next;
};


/**
 * Pool de `struct pop3', para ser reusados.
 *
 * Como tenemos un unico hilo que emite eventos no necesitamos barreras de
 * contención.
 */
static const unsigned  max_pool  = 50; // tamaño máximo
static unsigned        pool_size = 0;  // tamaño actual
static struct pop3 *     pool    = 0;  // pool propiamente dicho

static const struct state_definition * pop3_describe_states(void);

/* Crea un nuevo `struct pop3' */
static struct pop3 * pop3_new(int client_fd) {
    struct pop3 * ret;

    if(pool == NULL) {
        ret = malloc(sizeof(*ret));
    } else {
        ret       = pool;
        pool      = pool->next;
        ret->next = 0;
    }
    if(ret == NULL) {
        goto finally;
    }

    memset(ret, 0x00, sizeof(*ret));

    ret->origin_fd       = -1;
    ret->client_fd       = client_fd;
    ret->client_addr_len = sizeof(ret->client_addr);

    ret->stm    .initial   = ORIGIN_SERVER_RESOLUTION;
    ret->stm    .max_state = ERROR;
    ret->stm    .states    = pop3_describe_states();
    stm_init(&ret->stm);

    buffer_init(&ret->read_buffer,  N(ret->raw_buff_a), ret->raw_buff_a);
    buffer_init(&ret->write_buffer, N(ret->raw_buff_b), ret->raw_buff_b);

    pop3_session_init(&ret->session, false);

    ret->references = 1;
    finally:
    return ret;
}

/** realmente destruye */
static void pop3_destroy_(struct pop3 *s) {
    if(s->origin_resolution != NULL) {
        freeaddrinfo(s->origin_resolution);
        s->origin_resolution = 0;
    }
    free(s);
}

/**
 * destruye un  `struct pop3', tiene en cuenta las referencias
 * y el pool de objetos.
 */
static void pop3_destroy(struct pop3 *s) {
    if(s == NULL) {
        // nada para hacer
    } else if(s->references == 1) {
        if(s != NULL) {
            if(pool_size < max_pool) {
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
    struct pop3 *next, *s;
    for(s = pool; s != NULL ; s = next) {
        next = s->next;
        free(s);
    }
}

/** obtiene el struct (pop3 *) desde la llave de selección  */
#define ATTACHMENT(key) ( (struct pop3 *)(key)->data)

/* declaración forward de los handlers de selección de una conexión
 * establecida entre un cliente y el proxy.
 */
static void pop3_read(struct selector_key *key);
static void pop3_write(struct selector_key *key);
static void pop3_block(struct selector_key *key);
static void pop3_close(struct selector_key *key);

static const struct fd_handler pop3_handler = {
        .handle_read   = pop3_read,
        .handle_write  = pop3_write,
        .handle_close  = pop3_close,
        .handle_block  = pop3_block,
};

/** Intenta aceptar la nueva conexión entrante*/
void pop3_accept_connection(struct selector_key * key) {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct pop3 * state       = NULL;

    const file_descriptor client = accept(key->fd, (struct sockaddr *) &client_addr, &client_addr_len);

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

    if (SELECTOR_SUCCESS != selector_register(key->s, client, &pop3_handler, OP_WRITE, state)) {
        goto fail;
    }

    return ;

    fail:

    if(client != -1) {
        close(client);
    }

    pop3_destroy(state);
}

/** Used before changing state to set the interests of both ends (client_fd, origin_fd) */
selector_status set_interests(fd_selector s, int client_fd, int origin_fd, enum pop3_state state) {
    fd_interest client_interest = OP_NOOP, origin_interest = OP_NOOP;
    selector_status status = SELECTOR_SUCCESS;

    switch (state) {
        case ORIGIN_SERVER_RESOLUTION:
            break;
        case CONNECTING_TO_OS:
            break;
        case WELCOME_READ:
            origin_interest = OP_READ;
            break;
        case WELCOME_WRITE:
            client_interest = OP_WRITE;
            break;
        case REQUEST_READ:
            client_interest = OP_READ;
            break;
        case REQUEST_WRITE:
            origin_interest = OP_WRITE;
            break;
        case RESPONSE_READ:
            origin_interest = OP_READ;
            break;
        case RESPONSE_WRITE:
            client_interest = OP_WRITE;
            break;
        default:
            break;
    }

    status |= selector_set_interest(s, client_fd, client_interest);
    status |= selector_set_interest(s, origin_fd, origin_interest);

    return status;
}

////////////////////////////////////////////////////////////////////////////////
// ORIGIN SERVER RESOLUTION
////////////////////////////////////////////////////////////////////////////////

static void * os_resolution_blocking(void * data);

/*
 * Tenemos que resolver el nombre (operación bloqueante), por ende disparamos
 * la resolución en un thread que luego notificará al selector que ha terminado.
 */
unsigned origin_server_resolution(struct selector_key * key){
    pthread_t tid;
    /* Creamos una key para pasarle como argumento al thread */
    struct selector_key * k = malloc(sizeof(* key));

    if(k == NULL) {
       return ERROR;
    } else {
        memcpy(k, key, sizeof(*k));
        if(pthread_create(&tid, 0, os_resolution_blocking, k) == -1) {
            return ERROR;
        } else{
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
static void * os_resolution_blocking(void * data) {
    struct selector_key * key = (struct selector_key *) data;
    struct pop3 * s           = ATTACHMENT(key);

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

    char buff[7];
    snprintf(buff, sizeof(buff), "%hu",
             parameters->origin_port);
    printf("origin server: %s\n",parameters->origin_server);
    if (0 != getaddrinfo(parameters->origin_server, buff, &hints,
                         &s->origin_resolution)){
        //sprintf(stderr,"Domain name resolution error\n");
        printf("Domain name resolution error\n");
    }

    selector_notify_block(key->s, key->fd);

    free(data);

    return 0;
}

static unsigned origin_server_resolution_done(struct selector_key *key) {
    struct pop3 *s      =  ATTACHMENT(key);

    if(s->origin_resolution == 0) {
        // TODO: error
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

    if (SELECTOR_SUCCESS != selector_set_interest_key(key, OP_WRITE)) {
        return ERROR;
    }

    return CONNECTING_TO_OS;
}

////////////////////////////////////////////////////////////////////////////////
// CONNECTING_TO_OS
////////////////////////////////////////////////////////////////////////////////

int origin_connect(struct selector_key * key);

void send_error_(int fd, const char * error) {
    send(fd, error, strlen(error), 0);
}

/** intenta establecer una conexión con el origin server */
unsigned connect_(struct selector_key * key) {
    struct pop3 * d = ATTACHMENT(key);
    //TODO validar conexion exitosa
    if (origin_connect(key) == -1){
        send_error_(d->client_fd, "-ERR Connection to origin server failed\n");
        return ERROR;
    }

    bool pipelining = false;
    pop3_session_init(&ATTACHMENT(key)->session, pipelining);

    return WELCOME_READ;
}

// Connection utilities
int origin_connect(struct selector_key * key) {
    int sock = socket(ATTACHMENT(key)->origin_domain, SOCK_STREAM, IPPROTO_TCP);
    ATTACHMENT(key)->origin_fd = sock;

    printf("server socket: %d\n", sock);

    if (sock < 0) {
        perror("socket() failed");
        return sock;
    }

    if (selector_fd_set_nio(sock) == -1) {
        goto error;
    }

    /* Establish the connection to the origin server */
    if (-1 == connect(sock,
                      (const struct sockaddr *)&ATTACHMENT(key)->origin_addr,
                      ATTACHMENT(key)->origin_addr_len)) {
        if(errno == EINPROGRESS) {
            // es esperable,  tenemos que esperar a la conexión

            // dejamos de pollear el socket del cliente
            selector_status st = selector_set_interest_key(key, OP_NOOP);
            if(SELECTOR_SUCCESS != st) {
                goto error;
            }

            // esperamos la conexion en el nuevo socket
            st = selector_register(key->s, sock, &pop3_handler, OP_READ, key->data);
            if(SELECTOR_SUCCESS != st) {
                goto error;
            }
            ATTACHMENT(key)->references += 1;
        } else {
            goto error;
        }
    } else {
        // estamos conectados sin esperar... no parece posible
        abort();
    }

    return sock;

    error:
    if (sock != -1) {
        close(sock);
        ATTACHMENT(key)->origin_fd = -1;
    }

    return -1;
}

////////////////////////////////////////////////////////////////////////////////
// WELCOME
////////////////////////////////////////////////////////////////////////////////

/* Inicializa las variables de los estados WELCOME */
static void welcome_init(const unsigned state, struct selector_key *key) {
    struct welcome_st *d = &ATTACHMENT(key)->orig.welcome;
    d->wb = &(ATTACHMENT(key)->write_buffer);
}

/* Lee todos los bytes del mensaje de tipo `welcome' en server_fd */
static unsigned welcome_read(struct selector_key *key) {
    printf("EN WELCOME READ\n");
    struct welcome_st *d = &ATTACHMENT(key)->orig.welcome;
    unsigned  ret      = WELCOME_WRITE;
    uint8_t *ptr;
    size_t  count;
    ssize_t  n;

    ptr = buffer_write_ptr(d->wb, &count);
    const char * msg = "Proxy server POP3\r\n";
    n = strlen(msg);
    memccpy(ptr, msg, 0, count);
    buffer_write_adv(d->wb, n);

    ptr = buffer_write_ptr(d->wb, &count);
    n = recv(key->fd, ptr, count, 0);

    if(n > 0) {
        buffer_write_adv(d->wb, n);
        if (SELECTOR_SUCCESS != set_interests(key->s, ATTACHMENT(key)->client_fd, key->fd, WELCOME_WRITE)) {
            ret = ERROR;
        }
    } else {
        ret = ERROR;
    }

    return ret;
}

/** escribe todos los bytes del mensaje `welcome' en client_fd */
static unsigned welcome_write(struct selector_key *key) {
    printf("EN WELCOME WRITE\n");
    struct welcome_st *d = &ATTACHMENT(key)->orig.welcome;

    unsigned  ret      = WELCOME_WRITE;
    uint8_t *ptr;
    size_t  count;
    ssize_t  n;

    ptr = buffer_read_ptr(d->wb, &count);
    n = send(key->fd, ptr, count, MSG_NOSIGNAL);

    if(n == -1) {
        ret = ERROR;
    } else {
        buffer_read_adv(d->wb, n);
        if(!buffer_can_read(d->wb)) {
            // el server_fd ya esta en NOOP (seteado en welcome_read)
            if(SELECTOR_SUCCESS == selector_set_interest_key(key, OP_READ)) {
                ret = REQUEST_READ;
            } else {
                ret = ERROR;
            }
        }
    }


    return ret;
}

static void welcome_close(const unsigned state, struct selector_key *key) {
    printf("EN WELCOME CLOSE\n");
    struct welcome_st * d = &ATTACHMENT(key)->orig.welcome;
}


////////////////////////////////////////////////////////////////////////////////
// REQUEST
////////////////////////////////////////////////////////////////////////////////

static unsigned request_process(struct selector_key * key, struct request_st * d);

/* Inicializa las variables de los estados REQUEST */
static void request_init(const unsigned state, struct selector_key * key) {
    printf("EN REQUEST INIT\n");
    struct request_st * d = &ATTACHMENT(key)->client.request;

    d->rb              = &(ATTACHMENT(key)->read_buffer);
    d->wb              = &(ATTACHMENT(key)->write_buffer);
    d->parser.request  = &d->request;

    request_parser_init(&d->parser);
}

/* Lee la request del cliente */
static unsigned request_read(struct selector_key * key) {
    printf("EN REQUEST READ\n");
    uint8_t * ptr;
    size_t  count;
    ssize_t  n;
    struct request_st * d   = &ATTACHMENT(key)->client.request;
    unsigned ret            = REQUEST_READ;
    buffer * buffer_read    = d->rb;
    bool error              = false;

    ptr = buffer_write_ptr(buffer_read, &count);
    n   = recv(key->fd, ptr, count, 0);

    if(n > 0) {
        buffer_write_adv(buffer_read, n);
        while(ret == REQUEST_READ) {
            enum request_state st = request_consume(buffer_read, &d->parser, &error);
            if (request_is_done(st, 0)) {
                ret = request_process(key, d);
            }
        }
    } else if (!is_empty(ATTACHMENT(key)->session.request_queue)) {
        selector_status s = SELECTOR_SUCCESS;
        s |= selector_set_interest_key(key, OP_NOOP);
        s |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_WRITE);

        ret = SELECTOR_SUCCESS == s ? REQUEST_WRITE : ERROR;
    } else {
        ret = ERROR;
    }

    return error ? ERROR : ret;
}

/* Crea una request para ser encolada en la queue */
struct pop3_request * new_request(const struct pop3_request_cmd * cmd, char * args) {
    struct pop3_request * r = malloc(0);

    if (r == NULL) {
        return NULL;
    }

    r->cmd = cmd;
    r->args = args; // args ya fue alocado en el parser. se podria alocar aca tambien

    return r;
}

//TODO
void destroy_request(struct pop3_request *r) {
    free(r->args);
    free(r);
}

/* Procesa la request ya parseada */
static unsigned request_process(struct selector_key * key, struct request_st * d) {
    enum pop3_state ret;

    //TODO hay que limpiar el buffer
    switch (d->parser.state) {
        case request_error_inexistent_cmd:
            printf("unkown command\n");
            return ERROR;
        case request_error_cmd_too_long:
            printf("cmd too long\n");
            return ERROR;
        case request_error_param_too_long:
            printf("param too long\n");
            return ERROR;
    }

    buffer * b = d->wb;
    char * ptr;
    size_t  count;
    ssize_t  n;

    switch (d->request.cmd->id) {
        case error:
            //no la mandamos, le mandamos un mensaje de error al cliente y volvemos a leer de client_fd
            ptr = (char *)buffer_write_ptr(b, &count);
            sprintf(ptr, "BAD REQUEST: %s\n", d->parser.cmd_buffer);
            n = strlen(ptr);
            buffer_write_adv(b, n);
            set_interests(key->s, key->fd, ATTACHMENT(key)->origin_fd, RESPONSE_WRITE);
            return RESPONSE_WRITE;
        case retr:
        case user:
        case pass:
        case quit:
        default:
            break;
    }

    // encolamos la request
    struct pop3_request * r = new_request(d->request.cmd, d->request.args);

    if (r == NULL) {
        return ERROR;
    } else {
        enqueue(ATTACHMENT(key)->session.request_queue, r);
        //printf("queue size: %d\n", size(ATTACHMENT(key)->session.request_queue));
    }

    // todavia hay bytes por leer en el buffer
    if (buffer_can_read(d->rb)) {
        ret = REQUEST_READ;
    } else {
        ret = REQUEST_WRITE;
    }

    if (ret == REQUEST_WRITE) {
        selector_status s = SELECTOR_SUCCESS;
        s |= selector_set_interest_key(key, OP_NOOP);
        s |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_WRITE);

        ret = SELECTOR_SUCCESS == s ? REQUEST_WRITE : ERROR;
    }

    return ret;
}
/* Escrible la request en el server */
static unsigned request_write(struct selector_key *key) {
    printf("EN REQUEST WRITE\n");
    struct request_st *d = &ATTACHMENT(key)->client.request;

    unsigned  ret      = REQUEST_WRITE;
    buffer * b         = d->rb;
    uint8_t * ptr;
    size_t  count;
    ssize_t  n;

    // por ahora asumimos que el server no soporta pipelining entonces mandamos las requests de a una
    struct pop3_request * r = dequeue(ATTACHMENT(key)->session.request_queue);
    //printf("queue size: %d\n", size(ATTACHMENT(key)->session.request_queue));

    // todo ver una mejor forma de hacer esto (peek)
    d->request = *r;

    if (-1 == request_marshall(r, b)) {
        ret = ERROR;
    }

    ptr = buffer_read_ptr(b, &count);
    n = send(key->fd, ptr, count, MSG_NOSIGNAL);

    if(n == -1) {
        ret = ERROR;
    } else {
        buffer_read_adv(b, n);
        destroy_request(r);
        if(!buffer_can_read(b)) {
            // el client_fd ya esta en NOOP (seteado en request_read)
            if(SELECTOR_SUCCESS == selector_set_interest_key(key, OP_READ)) {
                ret = RESPONSE_READ;
            } else {
                ret = ERROR;
            }
        }
    }

    return ret;
}

static void request_close_(const unsigned state, struct selector_key *key) {
    struct request_st * d = &ATTACHMENT(key)->client.request;
}

////////////////////////////////////////////////////////////////////////////////
// RESPONSE
////////////////////////////////////////////////////////////////////////////////

static unsigned response_process(struct selector_key *key, struct request_st * d);

/* Lee la respuesta del origin server*/
static unsigned response_read(struct selector_key *key) {
    printf("EN RESPONSE READ\n");
    struct request_st *d = &ATTACHMENT(key)->client.request;
    unsigned  ret      = RESPONSE_READ;
    bool  error        = false;
    uint8_t *ptr;
    size_t  count;
    ssize_t  n;

    ptr = buffer_write_ptr(d->wb, &count);
    n = recv(key->fd, ptr, count, 0);

    if(n > 0) {
        buffer_write_adv(d->wb, n);
        ret = response_process(key, &ATTACHMENT(key)->client.request);
    } else {
        ret = ERROR;
    }

    return error ? ERROR : ret;
}

static unsigned response_process(struct selector_key *key, struct request_st * d) {
    enum pop3_state ret = RESPONSE_WRITE;

    // llamamos a la funcion de ejcucion del comando
    // TODO solo llamar cuando la respuesta fue +OK
    d->request.cmd->fn(&d->request);

    if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_NOOP)) {
        if (SELECTOR_SUCCESS != selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_WRITE)) {
            ret = ERROR;
        }
    } else {
        ret = ERROR;
    }

    return ret;
}

/** Escrible la respuesta en el cliente */
static unsigned response_write(struct selector_key *key) {
    printf("EN RESPONSE WRITE\n");
    struct request_st *d = &ATTACHMENT(key)->client.request;

    enum pop3_state  ret      = RESPONSE_WRITE;
    uint8_t *ptr;
    size_t  count;
    ssize_t  n;

    ptr = buffer_read_ptr(d->wb, &count);
    n = send(key->fd, ptr, count, MSG_NOSIGNAL);

    if(n == -1) {
        ret = ERROR;
    } else {
        buffer_read_adv(d->wb, n);
        if (!buffer_can_read(d->wb)) {
            // TODO hay que hacer peek en request write y recien desencolarla aca, tiene mas sentido

            if (d->request.cmd->id == quit) {
                selector_set_interest_key(key, OP_NOOP);
                ret = DONE;
            } else
            if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_READ)) {
                ret = REQUEST_READ;
            } else {
                ret = ERROR;
            }
        }
    }

    return ret;
}

static void response_close(const unsigned state, struct selector_key *key) {
    struct request_st * d = &ATTACHMENT(key)->client.request;
}

/** definición de handlers para cada estado */
static const struct state_definition client_statbl[] = {
        {
                .state            = ORIGIN_SERVER_RESOLUTION,
                .on_write_ready   = origin_server_resolution,
                .on_block_ready   = origin_server_resolution_done,
        },{
                .state            = CONNECTING_TO_OS,
                .on_write_ready   = connect_,
        },{
                .state            = WELCOME_READ,
                .on_arrival       = welcome_init,
                .on_read_ready    = welcome_read,
        },{
                .state            = WELCOME_WRITE,
                .on_write_ready   = welcome_write,
                .on_departure     = welcome_close,
        },/*{
                .state            = CAPA,
                .on_arrival       = capa_init,
                .on_read_ready     = capa_read,
        },*/{
                .state            = REQUEST_READ,
                .on_arrival       = request_init,
                .on_read_ready    = request_read,
        },{
                .state            = REQUEST_WRITE,
                .on_write_ready   = request_write,
                .on_departure     = request_close_,
        },{
                .state            = RESPONSE_READ,
                .on_read_ready    = response_read,
        },{
                .state            = RESPONSE_WRITE,
                .on_write_ready   = response_write,
                .on_departure     = response_close,
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
        if (fds[i] != -1) {
            if (SELECTOR_SUCCESS != selector_unregister_fd(key->s, fds[i])) {
                abort();
            }
            close(fds[i]);
        }
    }
}
