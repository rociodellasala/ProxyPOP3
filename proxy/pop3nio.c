/**
 * pop3nio.c  - controla el flujo de un proxy POP3 (sockets no bloqueantes)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>
#include <memory.h>

#include "include/pop3_session.h"
#include "include/buffer.h"
#include "include/stm.h"

#include "include/pop3nio.h"
#include "include/input_parser.h"
#include "include/client_parser_request.h"
#include "include/client_parser_response.h"

#include "include/metrics.h"
#include "include/utils.h"


#define N(x) (sizeof(x)/sizeof((x)[0]))

/* Maquina de estados general */
enum pop3_state {
            //estado terminal (error)
            ERROR = -1,

            ORIGIN_SERVER_RESOLUTION,
            CONNECTING_TO_OS,
            WELCOME_READ,
            WELCOME_WRITE,
            CAPA,
            REQUEST,
            RESPONSE,

            // estado terminales (exito)
            DONE,

};

////////////////////////////////////////////////////////////////////
// Definición de variables para cada estado

/* Usado por WELCOME_READ y WELCOME_WRITE */
struct welcome_st {
    // buffer utilizado para I/O
    buffer *                    write_buffer;
};

/* Usado por REQUEST */
struct request_st {
    // buffer utilizado para I/O
    buffer *                    read_buffer;
    buffer *                    write_buffer;

    struct pop3_request         request;

    // parser
    struct request_parser       request_parser;

};

/* Usado por RESPONSE */
struct response_st {
    // buffer utilizado para I/O
    buffer *                    read_buffer;
    buffer *                    write_buffer;

    struct pop3_request *       request;

    // parser
    struct response_parser      response_parser;
};


/* Tamaño de los buffers de I/O */
/* TODO: ver de achicarlo */
#define BUFFER_SIZE 60

/*
 * Si bien cada estado tiene su propio struct que le da un alcance
 * acotado, disponemos de la siguiente estructura para hacer una única
 * alocación cuando recibimos la conexión.
 *
 * Se utiliza un contador de referencias (references) para saber cuando debemos
 * liberarlo finalmente, y un pool para reusar alocaciones previas.
 */
struct pop3 {
    // información del cliente
    struct sockaddr_storage       client_addr;
    int                           client_fd;

    // resolución de la dirección del origin server
    struct addrinfo              *origin_resolution;
    

    // información del origin server
    struct sockaddr_storage       origin_addr;
    socklen_t                     origin_addr_len;
    int                           origin_domain;
    int                           origin_fd;

    int                           extern_read_fd;
    int                           extern_write_fd;

    struct pop3_session           session;

    // maquinas de estados
    struct state_machine          stm;

    // estados para el client_fd
    union {
        struct request_st         request;
    } client;

    // estados para el origin_fd
    union {
        struct welcome_st         welcome;
        struct response_st        response;
    } orig;


    // buffers para ser usados read_buffer, write_buffer
    uint8_t raw_buff_a[BUFFER_SIZE], raw_buff_b[BUFFER_SIZE];
    buffer read_buffer, write_buffer;

    uint8_t raw_super_buffer[BUFFER_SIZE];
    buffer super_buffer;

    uint8_t raw_extern_read_buffer[BUFFER_SIZE];
    buffer extern_read_buffer;

    // cantidad de referencias a este objeto
    // si es uno se debe destruir
    unsigned references;

    // siguiente en el pool
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
static struct pop3 *   pool      = 0;  // pool propiamente dicho

static const struct state_definition * pop3_describe_states(void);

/* Crea un nuevo `struct pop3' */
static struct pop3 * pop3_new(int client_fd) {
    struct pop3 * pop3;

    if (pool == NULL) {
        pop3 = malloc(sizeof(*pop3));
    } else {
        pop3       = pool;
        pool       = pool->next;
        pop3->next = 0;
    }

    if (pop3 == NULL) {
        goto finally;
    }

    memset(pop3, 0x00, sizeof(*pop3));

    pop3->origin_fd         = -1;
    pop3->client_fd         = client_fd;

    pop3->stm    .initial   = ORIGIN_SERVER_RESOLUTION;
    pop3->stm    .max_state = DONE;
    pop3->stm    .states    = pop3_describe_states();

    stm_init(&pop3->stm);

    buffer_init(&pop3->read_buffer,  N(pop3->raw_buff_a), pop3->raw_buff_a);
    buffer_init(&pop3->write_buffer, N(pop3->raw_buff_b), pop3->raw_buff_b);

    buffer_init(&pop3->super_buffer,  N(pop3->raw_super_buffer), pop3->raw_super_buffer);
    buffer_init(&pop3->extern_read_buffer,  N(pop3->raw_extern_read_buffer), pop3->raw_extern_read_buffer);

    pop3_session_init(&pop3->session, false);

    pop3->references = 1;

    finally:
    return pop3;
}

/* Realmente destruye */
static void pop3_destroy_(struct pop3 * pop3) {
    if (pop3->origin_resolution != NULL) {
        freeaddrinfo(pop3->origin_resolution);
        pop3->origin_resolution = 0;
    }

    free(pop3);
}

/**
 * Destruye un `struct pop3', tiene en cuenta las referencias
 * y el pool de objetos.
 */
static void pop3_destroy(struct pop3 * pop3) {
    if (pop3 == NULL) {
        // nada para hacer
    } else if (pop3->references == 1) {
        if (pop3 != NULL) {
            if (pool_size < max_pool) {
                pop3->next = pool;
                pool    = pop3;
                pool_size++;
            } else {
                pop3_destroy_(pop3);
            }
        }
    } else {
        pop3->references -= 1;
    }
}

void pop3_pool_destroy(void) {
    struct pop3 * next, * current_pop3;
    for(current_pop3 = pool; current_pop3 != NULL ; current_pop3 = next) {
        next = current_pop3->next;
        free(current_pop3);
    }
}

/* Obtiene el struct (pop3 *) desde la llave de selección  */
#define ATTACHMENT(key) ((struct pop3 *)(key)->data)

/*
 * Declaración de los handlers de selección de una conexión
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
void pop3_accept_connection(struct selector_key * key) {
    struct sockaddr_storage     client_addr;
    socklen_t                   client_addr_len = sizeof(client_addr);
    struct pop3 *               state           = NULL;

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

////////////////////////////////////////////////////////////////////////////////
// ORIGIN SERVER RESOLUTION
////////////////////////////////////////////////////////////////////////////////

static void * origin_server_resolution_blocking(void * data);

static unsigned origin_server_connect(struct selector_key * key);

/*
 * Tenemos que resolver el nombre (operación bloqueante), por ende disparamos
 * la resolución en un thread que luego notificará al selector que ha terminado.
 */
unsigned origin_server_resolution(struct selector_key * key){
    pthread_t tid;
    struct selector_key * k = malloc(sizeof(*key));

    if (k == NULL) {
        return ERROR;
    } else {
        memcpy(k, key, sizeof(*k));
        if (pthread_create(&tid, 0, origin_server_resolution_blocking, k) == -1) {
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
static void * origin_server_resolution_blocking(void * data) {
    struct selector_key * key   = (struct selector_key *) data;
    struct pop3 *         pop3  = ATTACHMENT(key);

    pthread_detach(pthread_self());
    pop3->origin_resolution = 0;

    struct addrinfo hints = {
            .ai_family    = AF_UNSPEC,
            .ai_socktype  = SOCK_STREAM,
            .ai_flags     = AI_PASSIVE,
            .ai_protocol  = 0,
            .ai_canonname = NULL,
            .ai_addr      = NULL,
            .ai_next      = NULL,
    };

    char buff[7];
    snprintf(buff, sizeof(buff), "%hu", parameters->origin_port);
    /* TODO: ROMPE */
    if (0 != getaddrinfo(parameters->origin_server, buff, &hints, &pop3->origin_resolution)){
        fprintf(stderr,"Domain name resolution error\n");
    }

    selector_notify_block(key->s, key->fd);

    free(data);

    return 0;
}

static unsigned origin_server_resolution_done(struct selector_key * key) {
    struct pop3 * pop3 =  ATTACHMENT(key);

    if (pop3->origin_resolution == 0) {
        char * msg = "-ERR Invalid domain.\r\n";
        send(ATTACHMENT(key)->client_fd, msg, strlen(msg), 0);
        return ERROR;
    } else {
        pop3->origin_domain   = pop3->origin_resolution->ai_family;
        pop3->origin_addr_len = pop3->origin_resolution->ai_addrlen;
        memcpy(&pop3->origin_addr,
               pop3->origin_resolution->ai_addr,
               pop3->origin_resolution->ai_addrlen);
        freeaddrinfo(pop3->origin_resolution);
        pop3->origin_resolution = 0;
    }

    if (SELECTOR_SUCCESS != selector_set_interest_key(key, OP_WRITE)) {
        return ERROR;
    }

    return origin_server_connect(key);
}

static unsigned origin_server_connect(struct selector_key * key) {
    int sock = socket(ATTACHMENT(key)->origin_domain, SOCK_STREAM, IPPROTO_TCP);

    if (sock < 0) {
        perror("socket() failed");
        return ERROR;
    }

    if (selector_fd_set_nio(sock) == -1) {
        goto error;
    }

    /* Establish the connection to the origin server */
    if (connect(sock,(const struct sockaddr *)&ATTACHMENT(key)->origin_addr,
                      ATTACHMENT(key)->origin_addr_len) == -1) {
        if (errno == EINPROGRESS) {
            // es esperable,  tenemos que esperar a la conexión

            // dejamos de pollear el socket del cliente
            selector_status st = selector_set_interest_key(key, OP_NOOP);
            if(SELECTOR_SUCCESS != st) {
                goto error;
            }

            // esperamos la conexion en el nuevo socket
            st = selector_register(key->s, sock, &pop3_handler,
                                   OP_WRITE, key->data);
            if (SELECTOR_SUCCESS != st) {
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

    return CONNECTING_TO_OS;

    error:

    if (sock != -1) {
        close(sock);
        //ATTACHMENT(key)->origin_fd = -1;
    }

    return ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// CONNECTING TO ORIGIN SERVER
////////////////////////////////////////////////////////////////////////////////

void
connecting_init(const unsigned state, struct selector_key * key) {
    // nada por hacer
}

void send_error_(int fd, const char * error) {
    send(fd, error, strlen(error), 0);
}

unsigned connecting(struct selector_key *key) {
    int error;
    socklen_t len = sizeof(error);
    struct pop3 *d = ATTACHMENT(key);

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
    pop3_session_init(&ATTACHMENT(key)->session, false);

    selector_status ss = SELECTOR_SUCCESS;

    ss |= selector_set_interest_key(key, OP_READ);
    ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_NOOP);

    return SELECTOR_SUCCESS == ss ? WELCOME_READ : ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// WELCOME READ AND WELCOME WRITE
////////////////////////////////////////////////////////////////////////////////

/* Inicializa las variables del estado WELCOME */
static void welcome_init(const unsigned state, struct selector_key * key) {
    struct welcome_st * welcome = &ATTACHMENT(key)->orig.welcome;

    welcome->write_buffer = &(ATTACHMENT(key)->write_buffer);
}

/* Lee todos los bytes del mensaje de tipo `welcome' de server_fd */
static unsigned welcome_read(struct selector_key * key) {
    uint8_t * ptr;
    size_t  count;
    ssize_t  n;
    struct welcome_st * welcome = &ATTACHMENT(key)->orig.welcome;
    enum pop3_state stm_next_status  = WELCOME_WRITE;

    // proxy welcome message
    ptr = buffer_write_ptr(welcome->write_buffer, &count);
    n = recv(key->fd, ptr, count, 0);
    buffer_write_adv(welcome->write_buffer, n);

    if (n > 0) {
        buffer_write_adv(welcome->write_buffer, 0);
        selector_status ss = SELECTOR_SUCCESS;
        ss |= selector_set_interest_key(key, OP_NOOP);
        ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_WRITE);
        if (ss != SELECTOR_SUCCESS) {
            stm_next_status = ERROR;
        }
    } else {
        stm_next_status = ERROR;
    }

    return stm_next_status;
}

/* Escribe todos los bytes del mensaje `welcome' en client_fd */
static unsigned welcome_write(struct selector_key * key) {
    uint8_t * ptr;
    size_t  count;
    ssize_t  n;
    struct welcome_st * welcome = &ATTACHMENT(key)->orig.welcome;
    enum pop3_state stm_next_status  = WELCOME_WRITE;

    ptr = buffer_read_ptr(welcome->write_buffer, &count);
    n = send(key->fd, ptr, count, MSG_NOSIGNAL);

    if(n == -1) {
        stm_next_status = ERROR;
    } else {
        buffer_read_adv(welcome->write_buffer, n);
        if(!buffer_can_read(welcome->write_buffer)) {
            selector_status ss = SELECTOR_SUCCESS;
            ss |= selector_set_interest_key(key, OP_NOOP);
            ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_READ);
            stm_next_status = SELECTOR_SUCCESS == ss ? CAPA : ERROR;
        }
    }

    return stm_next_status;
}

static void welcome_close(const unsigned state, struct selector_key *key) {
    //nada por hacer
}

////////////////////////////////////////////////////////////////////////////////
// CAPA
////////////////////////////////////////////////////////////////////////////////

void send_capability_to_origin_server(struct selector_key * key);
void set_pipelining(struct selector_key * key, struct response_st * d);

void capa_init(const unsigned state, struct selector_key * key) {
    struct response_st * response = &ATTACHMENT(key)->orig.response;
    struct pop3_request * pop3_request   = new_request(get_cmd("capa"), NULL);
    
    response->read_buffer                = &ATTACHMENT(key)->write_buffer;
    response->write_buffer               = &ATTACHMENT(key)->super_buffer;
    response->request                    = pop3_request;
    response->response_parser.request    = response->request;
    response_parser_init(&response->response_parser);
    
    send_capability_to_origin_server(key);
}

void send_capability_to_origin_server(struct selector_key * key) {
    char * msg = "CAPA\r\n";
    send(ATTACHMENT(key)->origin_fd, msg, strlen(msg), 0);
}

/** Lee la respuesta al comando capa */
static unsigned capa_read(struct selector_key * key) {
    struct response_st *d = &ATTACHMENT(key)->orig.response;
    enum pop3_state stm_next_status = CAPA;

    bool  error        = false;

    buffer  *b         = d->read_buffer;
    uint8_t *ptr;
    size_t  count;
    ssize_t  n;

    ptr = buffer_write_ptr(b, &count);
    n = recv(key->fd, ptr, count, 0);

    if(n > 0) {
        buffer_write_adv(b, n);
        response_consume(b, d->write_buffer, &d->response_parser, &error);
        d->response_parser.first_line_done = false;
        enum response_state st = response_consume(b, d->write_buffer, &d->response_parser, &error);
        if (response_is_done(st, 0)) {
            set_pipelining(key, d);
            selector_status ss = SELECTOR_SUCCESS;
            ss |= selector_set_interest_key(key, OP_NOOP);
            ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_READ);
            stm_next_status= SELECTOR_SUCCESS == ss ? REQUEST : ERROR;
        }
    } else {
        stm_next_status= ERROR;
    }

    return stm_next_status;
}

static void capa_close(const unsigned state, struct selector_key * key) {
    //nada por hacer
}

char * to_upper(char * str) {
    char * aux = str;
    while (*aux != 0) {
        *aux = (char)toupper(*aux);
        aux++;
    }

    return str;
}

void set_pipelining(struct selector_key * key, struct response_st * response) {
    to_upper(response->response_parser.capa_response);

    char * capabilities = response->response_parser.capa_response;
    char * needle = "PIPELINING";

    struct pop3 * pop3 = ATTACHMENT(key);

    if (strstr(capabilities, needle) != NULL) {
        pop3->session.pipelining = true;
    } else {
        pop3->session.pipelining = false;
    }

    while (buffer_can_read(response->write_buffer)) {
        buffer_read(response->write_buffer);
    }

}

////////////////////////////////////////////////////////////////////////////////
// REQUEST
////////////////////////////////////////////////////////////////////////////////

enum pop3_state request_process(struct selector_key * key, struct request_st * request);

/* Inicializa las variables de los estados REQUEST y RESPONSE */
static void request_init(const unsigned state, struct selector_key * key) {
    struct request_st * request = &ATTACHMENT(key)->client.request;

    request->read_buffer                = &(ATTACHMENT(key)->read_buffer);
    request->write_buffer               = &(ATTACHMENT(key)->write_buffer);
    request->request_parser.request     = &request->request;

    request_parser_init(&request->request_parser);
}

/* Lee todos los bytes del mensaje de tipo `request' y inicia su proceso */
static unsigned request_read(struct selector_key * key) {
    uint8_t * ptr;
    size_t  count;
    ssize_t  n;
    struct request_st * request = &ATTACHMENT(key)->client.request;
    enum pop3_state stm_next_status  = REQUEST;
    buffer * read_buffer        = request->read_buffer;
    bool error                  = false;

    ptr = buffer_write_ptr(read_buffer, &count);
    n = recv(key->fd, ptr, count, 0);

    if (n > 0) {
        buffer_write_adv(read_buffer, n);
        enum request_state st = request_consume(read_buffer, &request->request_parser, &error);
        if (request_is_done(st, 0)) {
            stm_next_status = request_process(key, request);
        }
    } else {
        stm_next_status = ERROR;
    }

    return stm_next_status;
}

/* Procesa la request previamente parseada y si es correcta la encola */
enum pop3_state request_process(struct selector_key * key, struct request_st * request) {
    enum pop3_state stm_next_status = REQUEST;

    /* todo: handlear aca los casos de mandar request fuera de state ? */
    if (request->request_parser.state >= request_error_inexistent_cmd) {
        char * dest;
        char * msg;
        switch (request->request_parser.state) {
            case request_error_inexistent_cmd:
                msg = "-ERR: Unknown command ";
                dest = append_cmd(dest, msg, request->request_parser.request->cmd->name);
                break;
            case request_error_cmd_too_long:
                dest = "-ERR: Command too long\r\n";
                break;
            case request_error_param_too_long:
                dest = "-ERR: Parameter too long\r\n";
                break;
            default:
                break;
        }

        send(key->fd, dest, strlen(dest), 0);

        ATTACHMENT(key)->session.concurrent_invalid_commands++;
        int cic = ATTACHMENT(key)->session.concurrent_invalid_commands;
        if (cic >= MAX_CONCURRENT_INVALID_COMMANDS) {
            msg = "-ERR: Too many invalid commands.\n";
            send(key->fd, msg, strlen(msg), 0);
            return DONE;
        }

        //reseteamos el parser
        request_parser_init(&request->request_parser);
        return REQUEST;
    }

    ATTACHMENT(key)->session.concurrent_invalid_commands = 0;

    // si la request es valida la encolamos
    struct pop3_request *r = new_request(request->request.cmd,request->request.args);
    if (r == NULL) {
        fprintf(stderr, "Memory error");
        return ERROR;
    }

    // encolo la request
    enqueue(ATTACHMENT(key)->session.request_queue, r);
    // reseteamos el parser
    request_parser_init(&request->request_parser);


    // no hay mas requests por leer, entonces vamos a request write
    if (!buffer_can_read(request->read_buffer)) {
        selector_status s = SELECTOR_SUCCESS;
        s |= selector_set_interest_key(key, OP_NOOP);
        s |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_WRITE);

        stm_next_status = SELECTOR_SUCCESS == s ? stm_next_status : ERROR;
    }

    return stm_next_status;
}

/* Escribe la request en el server */
static unsigned request_write(struct selector_key * key) {
    struct request_st * request = &ATTACHMENT(key)->client.request;

    unsigned  stm_next_status     = REQUEST;
    buffer * b         = request->write_buffer;
    uint8_t * ptr;
    size_t  count;
    ssize_t  n;

    //struct pop3_request *r = &d->request;

    // recorro la queue sin desencolar nada
    struct queue * queue = ATTACHMENT(key)->session.request_queue;
    struct pop3_request * pop3_request;

    //si el server no soporta pipelining solo mando la primer request
    if (ATTACHMENT(key)->session.pipelining == false) {
        pop3_request = peek_data(queue);
        if (pop3_request == NULL) {
            fprintf(stderr, "Error empty queue");
            return ERROR;
        }
        // copio la request en el buffer
        if (-1 == request_marshall(pop3_request, b)) {
            stm_next_status= ERROR;
        }
    } else {
        // si el server soporta pipelining copio el resto de las requests y las mando todas juntas
        while ((pop3_request = queue_get_next(queue)) != NULL) {
            //printf("%s\n", r->cmd->name);
            if (-1 == request_marshall(pop3_request, b)) {
                fprintf(stderr, "Request buffer error");
                return ERROR;
            }
        }
    }

    ptr = buffer_read_ptr(b, &count);
    n = send(key->fd, ptr, count, MSG_NOSIGNAL);

    if (n == -1) {
        stm_next_status= ERROR;
    } else {
        buffer_read_adv(b, n);
        if (!buffer_can_read(b)) {
            // el client_fd ya esta en NOOP (seteado en request_read)
            if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_READ)) {
                stm_next_status= RESPONSE;
            } else {
                stm_next_status= ERROR;
            }
        }
    }

    return stm_next_status;
}

static void request_close(const unsigned state, struct selector_key * key) {
    //struct request_st * request = &ATTACHMENT(key)->client.request;
    //destroy_request(request->request_parser.request);
    //destroy_request(&request->request);
}

//////////////////////////////////////////////////////////////////////////////// ///////////////////////////////////////////////////////////////////////
// RESPONSE
////////////////////////////////////////////////////////////////////////////////

enum pop3_state response_process(struct selector_key *key, struct response_st * d);

void set_request(struct response_st *d, struct pop3_request *request) {
    if (request == NULL) {
        fprintf(stderr, "Request is NULL");
        abort();
    }
    d->request                  = request;
    d->response_parser.request  = request;
}

void
response_init(const unsigned state, struct selector_key *key) {
    struct response_st * d  = &ATTACHMENT(key)->orig.response;

    d->read_buffer          = &ATTACHMENT(key)->write_buffer;
    d->write_buffer         = &ATTACHMENT(key)->super_buffer;

    // desencolo una request
    set_request(d, dequeue(ATTACHMENT(key)->session.request_queue));
    response_parser_init(&d->response_parser);
}

enum pop3_state
response_process_capa(struct response_st *d) {
    // busco pipelinig
    to_upper(d->response_parser.capa_response);
    char * capabilities = d->response_parser.capa_response;
    // siempre pasar la needle en upper case
    char * needle = "PIPELINING";

    if (strstr(capabilities, needle) != NULL) {
        return RESPONSE;
    }

    // else
    size_t capa_length = strlen(capabilities);
    size_t needle_length = strlen(needle);

    char * eom = "\r\n.\r\n";
    size_t eom_length = strlen(eom);

    char * new_capa = calloc(capa_length - 3 + needle_length + eom_length + 1, sizeof(char));
    if (new_capa == NULL) {
        return ERROR;
    }
    // copio to-do menos los ultimos 3 caracteres
    memcpy(new_capa, capabilities, capa_length - 3);
    // agrego la needle
    memcpy(new_capa + capa_length - 3, needle, needle_length);
    // agrego eom
    memcpy(new_capa + capa_length - 3 + needle_length, eom, eom_length);

    //printf("--%s--", new_capa);

    free(capabilities);

    d->response_parser.capa_response = new_capa;

    //leer el buffer y copiar la nueva respuesta
    while (buffer_can_read(d->write_buffer)) {
        buffer_read(d->write_buffer);
    }

    uint8_t *ptr1;
    size_t   count1;

    ptr1 = buffer_write_ptr(d->write_buffer, &count1);
    strcpy((char *)ptr1, new_capa);
    buffer_write_adv(d->write_buffer, strlen(new_capa));

    return RESPONSE;
}

/**
 * Lee la respuesta del origin server. Si la respuesta corresponde al comando retr y se cumplen las condiciones,
 *  se ejecuta una transformacion externa
 */
static unsigned
response_read(struct selector_key *key) {
    struct response_st *d = &ATTACHMENT(key)->orig.response;
    unsigned  stm_next_status     = RESPONSE;
    bool  error        = false;

    buffer  *b         = d->read_buffer;
    uint8_t *ptr;
    size_t  count;
    ssize_t  n;

    ptr = buffer_write_ptr(b, &count);
    n = recv(key->fd, ptr, count, 0);

    if(n > 0 || buffer_can_read(b)) {
        buffer_write_adv(b, n);
        enum response_state st = response_consume(b, d->write_buffer, &d->response_parser, &error);

        // se termino de leer la primera linea
        if (d->response_parser.first_line_done) {
            d->response_parser.first_line_done = false;

            // si el comando era un retr y se cumplen las condiciones, disparamos la transformacion externa
            if (st == response_mail && d->request->response->status == response_status_ok
                && d->request->cmd->id == retr) {
                ;

            }

            //consumimos el resto de la respuesta
            st = response_consume(b, d->write_buffer, &d->response_parser, &error);
        }

        selector_status ss = SELECTOR_SUCCESS;
        ss |= selector_set_interest_key(key, OP_NOOP);
        ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_WRITE);
        stm_next_status= ss == SELECTOR_SUCCESS ? RESPONSE : ERROR;

        if (stm_next_status== RESPONSE && response_is_done(st, 0)) {
            if (d->request->cmd->id == capa) {
                response_process_capa(d);
            }
        }
    } else if (n == -1){
        stm_next_status= ERROR;
    }

    return error ? ERROR : stm_next_status;
}

/* Escribe la respuesta en el cliente */
static unsigned
response_write(struct selector_key *key) {
    struct response_st *d = &ATTACHMENT(key)->orig.response;

    enum pop3_state  stm_next_status= RESPONSE;

    buffer *b = d->write_buffer;
    uint8_t *ptr;
    size_t  count;
    ssize_t  n;

    ptr = buffer_read_ptr(b, &count);
    n = send(key->fd, ptr, count, MSG_NOSIGNAL);

    if(n == -1) {
        stm_next_status= ERROR;
    } else {
        buffer_read_adv(b, n);
        if (!buffer_can_read(b)) {
            if (d->response_parser.state != response_done) {
                if (d->request->cmd->id == retr)
                    metric_add_transfered_bytes(n);
                selector_status ss = SELECTOR_SUCCESS;
                ss |= selector_set_interest_key(key, OP_NOOP);
                ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_READ);
                stm_next_status= ss == SELECTOR_SUCCESS ? RESPONSE : ERROR;
            } else {
                if (d->request->cmd->id == retr)
                    ;
                stm_next_status= response_process(key, d);
            }
        }
    }

    return stm_next_status;
}

enum pop3_state
response_process(struct selector_key *key, struct response_st * d) {
    enum pop3_state stm_next_status;

    switch (d->request->cmd->id) {
        case quit:
            selector_set_interest_key(key, OP_NOOP);
            ATTACHMENT(key)->session.state = POP3_UPDATE;
            return DONE;
        case user:
            ATTACHMENT(key)->session.user = d->request->args;
            break;
        case pass:
            if (d->request->response->status == response_status_ok)
                ATTACHMENT(key)->session.state = POP3_TRANSACTION;
            break;
        case capa:
            break;
        default:
            break;
    }

    // si quedan mas requests/responses por procesar
    struct queue *q = ATTACHMENT(key)->session.request_queue;
    if (!is_empty(q)) {
        // vuelvo a response_read porque el server soporta pipelining entonces ya le mande to-do y espero respuestas
        if (ATTACHMENT(key)->session.pipelining) {
            set_request(d, dequeue(q));
            response_parser_init(&d->response_parser);

            selector_status ss = SELECTOR_SUCCESS;
            ss |= selector_set_interest_key(key, OP_NOOP);
            ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_READ);
            stm_next_status= ss == SELECTOR_SUCCESS ? RESPONSE : ERROR;
        } else {
            //vuelvo a request write, hay request encoladas que todavia no se mandaron
            selector_status ss = SELECTOR_SUCCESS;
            ss |= selector_set_interest_key(key, OP_NOOP);
            ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_WRITE);
            stm_next_status= ss == SELECTOR_SUCCESS ? REQUEST : ERROR;
        }

    } else {
        // voy a request read
        selector_status ss = SELECTOR_SUCCESS;
        ss |= selector_set_interest_key(key, OP_READ);
        ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_NOOP);
        stm_next_status = ss == SELECTOR_SUCCESS ? REQUEST : ERROR;
    }

    return stm_next_status;
}

static void
response_close(const unsigned state, struct selector_key *key) {
    struct response_st * d = &ATTACHMENT(key)->orig.response;
    response_parser_close(&d->response_parser);
}

/* Definición de handlers para cada estado */
static const struct state_definition client_states[] = {
        {
                .state            = ORIGIN_SERVER_RESOLUTION,
                .on_write_ready   = origin_server_resolution,
                .on_block_ready   = origin_server_resolution_done,
        },{
                .state            = CONNECTING_TO_OS,
                .on_arrival       = connecting_init,
                .on_write_ready   = connecting,
        },{
                .state            = WELCOME_READ,
                .on_arrival       = welcome_init,
                .on_read_ready    = welcome_read,
        },{
                .state            = WELCOME_WRITE,
                .on_write_ready   = welcome_write,
                .on_departure     = welcome_close,
        },{
                .state            = CAPA,
                .on_arrival       = capa_init,
                .on_read_ready    = capa_read,
                .on_departure     = capa_close,
        },{
                .state            = REQUEST,
                .on_arrival       = request_init,
                .on_read_ready    = request_read,
                .on_write_ready   = request_write,
                .on_departure     = request_close,
        },{
                .state            = RESPONSE,
                .on_arrival       = response_init,
                .on_read_ready    = response_read,
                .on_write_ready   = response_write,
                .on_departure     = response_close,
        },{
                .state            = DONE,

        },{
                .state            = ERROR,
        }
};

static const struct state_definition * pop3_describe_states(void) {
    return client_states;
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
