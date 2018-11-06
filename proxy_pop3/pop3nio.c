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
#include <netdb.h>

#include "include/pop3_handler.h"
#include "include/pop3_session.h"
#include "include/buffer.h"
#include "include/stm.h"
#include "include/pop3nio.h"
#include "include/input_parser.h"
#include "include/client_parser_request.h"
#include "include/client_parser_response.h"
#include "include/metrics.h"
#include "include/utils.h"
#include "include/pipelining.h"
#include "include/client_request_utils.h"
#include "include/pop3_multi.h"
#include "include/client_response_utils.h"


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

    pop3->references = 1;

    finally:
    return pop3;
}

static void pop3_destroy_(struct pop3 * pop3) {
    if (pop3->origin_resolution != NULL) {
        freeaddrinfo(pop3->origin_resolution);
        pop3->origin_resolution = 0;
    }

    free(pop3);
}

void pop3_destroy(struct pop3 * pop3) {
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

    for (current_pop3 = pool; current_pop3 != NULL ;current_pop3 = next) {
        next = current_pop3->next;
        free(current_pop3);
    }
}


/*
 * Intenta aceptar la nueva conexión entrante
 */
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

    state = pop3_new(client);

    if (state == NULL) {
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

/*
 *      ESTADO: ORIGIN SERVER RESOLUTION
 *
 *      Se hace una resolucion DNS del address del origin server a traves
 *      de un thread para que no se bloquee
 *
 */
static enum pop3_state origin_server_connect(struct selector_key * key);
static void * origin_server_resolution_blocking(void * data);
/*
 *  Tenemos que resolver el nombre (operación bloqueante), por ende disparamos
 *  la resolución en un thread que luego notificará al selector que ha terminado.
 */
int origin_server_resolution(struct selector_key * key){
    pthread_t tid;
    struct selector_key * k               = malloc(sizeof(*key));
    enum pop3_state       stm_next_status = ORIGIN_SERVER_RESOLUTION;

    if (k == NULL) {
        stm_next_status = ERROR;
    } else {
        memcpy(k, key, sizeof(*k));
        if (pthread_create(&tid, 0, origin_server_resolution_blocking, k) == -1) {
            stm_next_status = ERROR;
        } else{
            selector_set_interest_key(key, OP_NOOP);
        }
    }

    return stm_next_status;
}

/**
 * Realiza la resolución de DNS bloqueante
 *
 * Una vez resuelto notifica al selector para que el evento esté
 * disponible en la próxima iteración
 */
static void * origin_server_resolution_blocking(void * data) {
    char                    service[7];
    struct selector_key *   key   = (struct selector_key *) data;
    struct pop3 *           pop3  = ATTACHMENT(key);

    pthread_detach(pthread_self());

    pop3->origin_resolution = 0;

    struct addrinfo hints = {
            .ai_family    = AF_UNSPEC,    /* Allow IPv4 or IPv6 */
            .ai_socktype  = SOCK_STREAM,  /* Datagram socket */
            .ai_flags     = AI_PASSIVE,   /* For wildcard IP address */
            .ai_protocol  = 0,            /* Any protocol */
            .ai_canonname = NULL,
            .ai_addr      = NULL,
            .ai_next      = NULL,
    };

    snprintf(service, sizeof(service), "%hu", parameters->origin_port);

    if (getaddrinfo(parameters->origin_server, service, &hints, &pop3->origin_resolution) != 0){
        fprintf(stderr,"Domain name resolution error\n");
    }

    selector_notify_block(key->s, key->fd);
    free(data);

    return 0;
}

int origin_server_resolution_done(struct selector_key * key) {
    struct pop3 * pop3 =  ATTACHMENT(key);

    if (pop3->origin_resolution == 0) {
        char * msg = "-ERR Invalid address.\r\n";
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

static enum pop3_state origin_server_connect(struct selector_key * key) {
    file_descriptor sock            = socket(ATTACHMENT(key)->origin_domain, SOCK_STREAM, IPPROTO_TCP);
    enum pop3_state stm_next_status = CONNECTING_TO_OS;

    if (sock < 0) {
        perror("socket() failed");
        return ERROR;
    }

    if (selector_fd_set_nio(sock) == -1) {
        goto error;
    }

    if (connect(sock,(const struct sockaddr *)&ATTACHMENT(key)->origin_addr,
                ATTACHMENT(key)->origin_addr_len) == -1) {
        if (errno == EINPROGRESS) {

            selector_status st = selector_set_interest_key(key, OP_NOOP);

            if (SELECTOR_SUCCESS != st) {
                goto error;
            }

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
        abort();
    }

    return stm_next_status;

    error:

    stm_next_status = ERROR;

    if (sock != -1) {
        close(sock);
    }

    return stm_next_status;
}

/*
 *    ESTADO: CONNECTING TO ORIGIN SERVER
 *
 *    Nos conectamos al servidor origen
 *
 */
int connecting(struct selector_key * key) {
    int error;
    socklen_t       len         = sizeof(error);
    struct pop3 *   d           = ATTACHMENT(key);
    char *          error_msg   = "-ERR Connection refused.\r\n";

    d->origin_fd = key->fd;

    if (getsockopt(key->fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        send(d->client_fd, error_msg, strlen(error_msg), 0);
        fprintf(stderr, "Connection failed\n");
        selector_set_interest_key(key, OP_NOOP);
        return ERROR;
    } else {
        if (error == 0) {
            d->origin_fd = key->fd;
        } else {
            send(d->client_fd, error_msg, strlen(error_msg), 0);
            fprintf(stderr, "Connection failed\n");
            selector_set_interest_key(key, OP_NOOP);
            return ERROR;
        }
    }

    pop3_session_init(&ATTACHMENT(key)->session, false);

    selector_status ss = SELECTOR_SUCCESS;
    ss |= selector_set_interest_key(key, OP_READ);
    ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_NOOP);

    return SELECTOR_SUCCESS == ss ? WELCOME_READ : ERROR;
}

/*
 *    ESTADO: WELCOME READ
 *
 *    Recibe el mensaje de bienvenida del origin server
 *
 */
static void welcome_init(struct selector_key * key) {
    struct welcome_st * welcome = &ATTACHMENT(key)->orig.welcome;

    welcome->buffer             = &ATTACHMENT(key)->write_buffer;
}

static int welcome_read(struct selector_key * key) {
    uint8_t *   ptr;
    ssize_t     count;
    ssize_t     n;

    struct welcome_st * welcome         = &ATTACHMENT(key)->orig.welcome;
    enum pop3_state     stm_next_status = WELCOME_WRITE;

    // pop3filter welcome message
    ptr = buffer_write_ptr((buffer *) welcome->buffer, (size_t *) &count);
    n   = recv(key->fd, ptr, (size_t) count, 0);

    if (count == n) {
        welcome->msg_not_finished = true;
    }

    if (n < count) {
        welcome->msg_not_finished = false;
    }

    if (n > 0) {
        buffer_write_adv(welcome->buffer, n);
        
        selector_status ss = SELECTOR_SUCCESS;
        ss |= selector_set_interest_key(key, OP_NOOP);
        ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_WRITE);
        
        stm_next_status = SELECTOR_SUCCESS == ss ? stm_next_status : ERROR;
    } else {
        stm_next_status = ERROR;
    }

    return stm_next_status;
}


/*
 *    ESTADO: WELCOME WRITE
 *
 *    Recibe el mensaje de bienvenida del origin server
 *
 */
static int welcome_write(struct selector_key * key) {
    uint8_t *   ptr;
    size_t      count;
    ssize_t     n;

    struct welcome_st * welcome         = &ATTACHMENT(key)->orig.welcome;
    enum pop3_state     stm_next_status = WELCOME_WRITE;

    ptr = buffer_read_ptr((buffer *) welcome->buffer, &count);
    n   = send(key->fd, ptr, count, MSG_NOSIGNAL);

    if (n == -1) {
        stm_next_status = ERROR;
    } else {
        buffer_read_adv((buffer *) welcome->buffer, n);

        if (welcome->msg_not_finished == true) {
            selector_status ss = SELECTOR_SUCCESS;
            ss |= selector_set_interest_key(key, OP_NOOP);
            ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_READ);
            
            stm_next_status = SELECTOR_SUCCESS == ss ? WELCOME_READ : ERROR;
        } else if (!buffer_can_read((buffer *) welcome->buffer)) {
            selector_status ss = SELECTOR_SUCCESS;
            ss |= selector_set_interest_key(key, OP_NOOP);
            ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_READ);
            
            stm_next_status = SELECTOR_SUCCESS == ss ? CAPA : ERROR;
        }

    }

    return stm_next_status;
}


/*
 *    ESTADO: CAPA
 *
 *    Envia un mensaje pidiendo las capacidades del servidor
 *
 */

void capa_init(struct selector_key * key) {
    struct response_st *    response        = &ATTACHMENT(key)->orig.response;
    struct pop3_request *   pop3_request    = new_request(get_cmd("capa"), NULL);

    response->read_buffer                = &ATTACHMENT(key)->write_buffer;
    response->write_buffer               = &ATTACHMENT(key)->super_buffer;
    response->request                    = pop3_request;
    response->response_parser.request    = response->request;
    response_parser_init(&response->response_parser);

    char * msg = "CAPA\r\n";
    send(ATTACHMENT(key)->origin_fd, msg, strlen(msg), 0);
}

static int capa_read(struct selector_key * key) {
    uint8_t * ptr;
    size_t    count;
    ssize_t   n;

    struct response_st *    response        = &ATTACHMENT(key)->orig.response;
    bool                    error           = false;
    struct pop3 *           d               = ATTACHMENT(key);
    buffer *                buffer          = response->read_buffer;
    enum pop3_state         stm_next_status = CAPA;

    ptr = buffer_write_ptr(buffer, &count);
    n   = recv(key->fd, ptr, count, 0);

    if(n > 0) {
        buffer_write_adv(buffer, n);
        enum response_state st = response_consume(buffer, response->write_buffer, &response->response_parser, &error);

        if(st == response_error) {
            char * error_msg = "-ERR Response error. Disconecting ...\r\n";
            send(d->client_fd, error_msg, strlen(error_msg), 0);
            fprintf(stderr, "Origin server send and inexistent command\n");
            selector_set_interest_key(key, OP_NOOP);
            stm_next_status = ERROR;
            //LOGUEAR ACAAAAAAAAAAAA!!
        } else {
            response->response_parser.first_line_consumed = false;
            st = response_consume(buffer, response->write_buffer, &response->response_parser, &error);

            if (st == response_error) {
                char * error_msg = "-ERR Response error. Disconecting ...\r\n";
                send(d->client_fd, error_msg, strlen(error_msg), 0);
                fprintf(stderr, "Origin server send and inexistent command\n");
                selector_set_interest_key(key, OP_NOOP);
                stm_next_status = ERROR;
            } else {
                if (response_is_done(st, 0)) {
                    struct pop3 *pop3 = ATTACHMENT(key);
                    pop3->session.pipelining = is_pipelining_available(response->response_parser.capa_response);

                    while (buffer_can_read(response->write_buffer)) {
                        buffer_read(response->write_buffer);
                    }

                    selector_status ss = SELECTOR_SUCCESS;
                    ss |= selector_set_interest_key(key, OP_NOOP);
                    ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_READ);

                    stm_next_status = SELECTOR_SUCCESS == ss ? REQUEST : ERROR;
                }
            }
        }
    } else {
        stm_next_status = ERROR;
    }

    return stm_next_status;
}

void capa_close(struct selector_key * key) {
    free(ATTACHMENT(key)->orig.response.request);
    free(ATTACHMENT(key)->orig.response.response_parser.capa_response);
}



/*
 *    ESTADO: REQUEST READ
 *
 *
 *
 */
static void request_init(struct selector_key * key) {
    struct request_st * request = &ATTACHMENT(key)->client.request;

    request->read_buffer                = &(ATTACHMENT(key)->read_buffer);
    request->write_buffer               = &(ATTACHMENT(key)->write_buffer);
    request->request_parser.request     = &request->request;

    request_parser_reset(&request->request_parser);
}

static int request_read(struct selector_key * key) {
    uint8_t *   ptr;
    size_t      count;
    ssize_t     n;

    struct request_st * request         = &ATTACHMENT(key)->client.request;
    buffer *            read_buffer     = request->read_buffer;
    bool                error           = false;
    enum pop3_state     stm_next_status = REQUEST;

    ptr = buffer_write_ptr(read_buffer, &count);
    n   = recv(key->fd, ptr, count, 0);

    if (n > 0 || buffer_can_read(read_buffer)) {
        buffer_write_adv(read_buffer, n);
        
        enum request_state st = request_consume(read_buffer, &request->request_parser, &error);
        
        if (request_is_done(st, 0)) {
            stm_next_status = process_request(key, request);
        }
        
    } else {
        stm_next_status = ERROR;
    }

    return stm_next_status;
}

static int request_write(struct selector_key * key) {
    uint8_t *   ptr;
    size_t      count;
    ssize_t     n;

    struct request_st *     request         = &ATTACHMENT(key)->client.request;
    struct msg_queue *      queue           = ATTACHMENT(key)->session.request_queue;
    buffer *                buffer          = request->write_buffer;
    struct pop3_request *   pop3_request    = NULL;
    enum pop3_state         stm_next_status = REQUEST;

    while ((pop3_request = request_to_buffer(buffer, ATTACHMENT(key)->session.pipelining, pop3_request, queue)) != NULL) {
        ptr = buffer_read_ptr(buffer, &count);
        n   = send(key->fd, ptr, count, MSG_NOSIGNAL);
        
        buffer_read_adv(buffer, n);
    }

    ptr = buffer_read_ptr(buffer, &count);
    n   = send(key->fd, ptr, count, MSG_NOSIGNAL);

    if (n == -1) {
        stm_next_status = ERROR;
    } else {
        buffer_read_adv(buffer, n);
        
        if (!buffer_can_read(buffer)) {
            if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_READ)) {
                stm_next_status = RESPONSE;
            } else {
                stm_next_status = ERROR;
            }
        }
    }

    return stm_next_status;
}


////////////////////////////////////////////////////////////////////////////////
// RESPONSE
////////////////////////////////////////////////////////////////////////////////

void response_init(struct selector_key * key) {
    struct response_st * d  = &ATTACHMENT(key)->orig.response;

    d->read_buffer          = &ATTACHMENT(key)->write_buffer;
    d->write_buffer         = &ATTACHMENT(key)->super_buffer;

    // desencolo una request
    struct pop3_request * request = dequeue(ATTACHMENT(key)->session.request_queue);

    if (request == NULL) {
        fprintf(stderr, "Request is NULL");
        abort();
    }

    d->request                  = request;
    d->response_parser.request  = request;

    response_parser_init(&d->response_parser);
}

static int response_read(struct selector_key * key) {
    uint8_t *   ptr;
    size_t      count;
    ssize_t     n;

    struct response_st *    response_st     = &ATTACHMENT(key)->orig.response;
    bool                    error           = false;
    buffer *                buffer          = response_st->read_buffer;
    enum pop3_state         stm_next_status = RESPONSE;

    ptr = buffer_write_ptr(buffer, &count);
    n   = recv(key->fd, ptr, count, 0);

    if (n > 0 || buffer_can_read(buffer)) {
        buffer_write_adv(buffer, n);
        enum response_state st = response_consume(buffer, response_st->write_buffer, &response_st->response_parser, &error);
        if(st == response_error) {
            char *error_msg = "-ERR Response error. Disconecting ...\r\n";
            send(ATTACHMENT(key)->client_fd, error_msg, strlen(error_msg), 0);
            fprintf(stderr, "Origin server send and inexistent command\n");
            selector_set_interest_key(key, OP_NOOP);
            stm_next_status = ERROR;
        }
        else {if (response_st->response_parser.first_line_consumed) {
            response_st->response_parser.first_line_consumed = false;

            if (st == response_get_mail && response_st->request->response->status == response_status_ok
                && (response_st->request->cmd->id == retr || response_st->request->cmd->id == top)) {
                if (parameters->filter_command->switch_program && parameters->filter_command != NULL) {
                    selector_status ss = SELECTOR_SUCCESS;
                    ss |= selector_set_interest_key(key, OP_NOOP);
                    ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_NOOP);

                    while (buffer_can_read(response_st->write_buffer)) {
                        buffer_read(response_st->write_buffer);
                    }

                    return ss == SELECTOR_SUCCESS ? EXTERNAL_TRANSFORMATION : ERROR;
                }
            }

            st = response_consume(buffer, response_st->write_buffer, &response_st->response_parser, &error);
        }

        selector_status ss = SELECTOR_SUCCESS;
        ss |= selector_set_interest_key(key, OP_NOOP);
        ss |= selector_set_interest(key->s, ATTACHMENT(key)->client_fd, OP_WRITE);

        stm_next_status = ss == SELECTOR_SUCCESS ? RESPONSE : ERROR;

        if (stm_next_status == RESPONSE && response_is_done(st, 0)) {
            if (response_st->request->cmd->id == capa) {
                struct pop3 * pop3 = ATTACHMENT(key);
                pop3->session.pipelining = is_pipelining_available(response_st->response_parser.capa_response);
                stm_next_status = RESPONSE;
            }
        }}

    } else if (n == -1){
        stm_next_status= ERROR;
    }

    return error ? ERROR : stm_next_status;
}

static int response_write(struct selector_key * key) {
    uint8_t *   ptr;
    size_t      count;
    ssize_t     n;

    struct response_st *    response_st     = &ATTACHMENT(key)->orig.response;
    enum pop3_state         stm_next_status = RESPONSE;
    buffer *                buffer          = response_st->write_buffer;

    ptr = buffer_read_ptr(buffer, &count);
    n   = send(key->fd, ptr, count, MSG_NOSIGNAL);

    if (n == -1) {
        stm_next_status= ERROR;
    } else {
        buffer_read_adv(buffer, n);

        if (!buffer_can_read(buffer)) {
            if (response_st->response_parser.state != response_done) {
                if (response_st->request->cmd->id == retr) {
                    metric_add_transfered_bytes(n);
                }

                selector_status ss = SELECTOR_SUCCESS;
                ss |= selector_set_interest_key(key, OP_NOOP);
                ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_READ);

                stm_next_status = ss == SELECTOR_SUCCESS ? RESPONSE : ERROR;
            } else {
                //if (d->request->cmd->id == retr) { ;

                //}
                stm_next_status= process_response(key, response_st);
            }
        }
    }

    return stm_next_status;
}


void response_close(struct selector_key * key) {
    if(ATTACHMENT(key)->orig.response.request->cmd->id == capa){
        free(ATTACHMENT(key)->orig.response.response_parser.capa_response);
    }

    free(ATTACHMENT(key)->orig.response.request->args);
    free(ATTACHMENT(key)->orig.response.request);

}

////////////////////////////////////////////////////////////////////////////////
// EXTERNAL TRANSFORMATION
////////////////////////////////////////////////////////////////////////////////

/* Inicializa las variables del estado EXTERNAL_TRANSFORMATION */
static void external_transformation_init(struct selector_key *key) {
    struct external_transformation *et = &ATTACHMENT(key)->et;

    et->read_buffer  = &ATTACHMENT(key)->write_buffer;
    et->write_buffer = &ATTACHMENT(key)->extern_read_buffer;
    et->ext_rb       = &ATTACHMENT(key)->extern_read_buffer;
    et->ext_wb       = &ATTACHMENT(key)->write_buffer;

    et->origin_fd    = &ATTACHMENT(key)->origin_fd;
    et->client_fd    = &ATTACHMENT(key)->client_fd;
    et->ext_read_fd  = &ATTACHMENT(key)->extern_read_fd;
    et->ext_write_fd = &ATTACHMENT(key)->extern_write_fd;

    et->finish_rd    = false;
    et->finish_wr    = false;
    et->error_wr     = false;
    et->error_rd     = false;

    et->did_write    = false;
    et->write_error  = false;

    et->send_bytes_write    = 0;
    et->send_bytes_read     = 0;

    if (et->parser_read == NULL) {
        et->parser_read = parser_init(parser_no_classes(), pop3_multi_parser());
    }

    if (et->parser_write == NULL){
        et->parser_write = parser_init(parser_no_classes(), pop3_multi_parser());
    }

    parser_reset(et->parser_read);
    parser_reset(et->parser_write);

    et->status = start_external_transformation(key, &ATTACHMENT(key)->session);

    char *          ptr;
    size_t          count;
    buffer  *       buffer  = et->write_buffer;
    const char *    err_msg = "-ERR Could not open external transformation.\r\n";

    ptr = (char*) buffer_write_ptr(buffer, &count);

    if (et->status == et_status_err) {
        printf(ptr, err_msg);
        buffer_write_adv(buffer, strlen(err_msg));
        selector_set_interest(key->s, *et->client_fd, OP_WRITE);
    }

    buffer = et->read_buffer;

    ;//log_request(ATTACHMENT(key)->orig.response.request);
    if (parse_mail(buffer, et->parser_read, &et->send_bytes_read)){
        et->finish_rd = true;
    }
}


static int external_transformation_read(struct selector_key *key) {
    uint8_t * ptr;
    size_t    count;
    ssize_t   n;
    struct external_transformation * et     = &ATTACHMENT(key)->et;
    enum pop3_state                  ret    = EXTERNAL_TRANSFORMATION;
    buffer *                         buffer = et->read_buffer;

    ptr = buffer_write_ptr(buffer, &count);
    n   = recv(*et->origin_fd, ptr, count, 0);

    if(n > 0) {
        buffer_write_adv(buffer, n);
        if (parse_mail(buffer, et->parser_read, &et->send_bytes_read) || n == 0) {
            if(et->error_rd) {
                buffer_read_adv(buffer, et->send_bytes_read);
            }
            //log_response(ATTACHMENT(key)->orig.response.request->response);
            et->finish_rd = true;
            if (finished_et(et)){
                struct msg_queue * queue = ATTACHMENT(key)->session.request_queue;

                if (!is_empty(queue)) {
                    ;//log_response(ATTACHMENT(key)->orig.response.request->response);
                    selector_set_interest(key->s, *et->client_fd, OP_NOOP);
                    selector_set_interest(key->s, *et->origin_fd, OP_READ);
                    ret = RESPONSE;
                } else {
                    ;//log_response(ATTACHMENT(key)->orig.response.request->response);
                    selector_set_interest(key->s, *et->origin_fd, OP_NOOP);
                    selector_set_interest(key->s, *et->client_fd, OP_READ);
                    ret = REQUEST;
                }
            } else {
                selector_set_interest(key->s, *et->ext_write_fd, OP_WRITE);
                selector_set_interest(key->s, *et->origin_fd, OP_NOOP);
            }
            // et->status = et_status_done;
        } else {
            if (!et->error_rd) {
                selector_set_interest(key->s, *et->ext_write_fd, OP_WRITE);
                selector_set_interest(key->s, *et->origin_fd, OP_NOOP);
            } else {
                buffer_read_adv(buffer, n);
            }
        }
    } else if (n == -1) {
        ret = ERROR;
    }

    return ret;
}

//escribir en el cliente
static int external_transformation_write(struct selector_key * key) {
    struct external_transformation * et     = &ATTACHMENT(key)->et;
    enum pop3_state                  ret    = EXTERNAL_TRANSFORMATION;
    buffer  *                        buffer = et->write_buffer;
    uint8_t * ptr;
    size_t   count;
    ssize_t  n;

    if (et->error_wr && !et->did_write){
        et->write_error = true;
        buffer_reset(buffer);
        ptr             = buffer_write_ptr(buffer, &count);
        char * err_msg  = "-ERR Could not open transformation.\r\n";
        sprintf((char *) ptr, "%s", err_msg);
        buffer_write_adv(buffer, strlen(err_msg));
    }

    if (et->error_wr && et->did_write && !et->write_error){
        et->write_error = true;
        ptr             = buffer_write_ptr(buffer, &count);
        char * err_msg  = "\r\n.\r\n";
        sprintf((char *) ptr, "%s", err_msg);
        buffer_write_adv(buffer, strlen(err_msg));
    }

    ptr                 = buffer_read_ptr(buffer, &count);
    size_t bytes_sent   = count;

    if (et->send_bytes_write != 0) {
        bytes_sent = et->send_bytes_write;
    }

    n   = send(*et->client_fd, ptr, bytes_sent, 0);

    if(n > 0) {
        if (et->send_bytes_write != 0){
            et->send_bytes_write -= n;
            et->finish_wr = true;
        }
        et->did_write = true;
        buffer_read_adv(buffer, n);
        //if (et->finish_wr)
        ;//metricas->retrieved_messages++;
        if ((et->error_wr || et->finish_wr) && et->send_bytes_write == 0) {
            if (finished_et(et)) {
                struct msg_queue *q = ATTACHMENT(key)->session.request_queue;
                // vamos a response read, todavia hay respuestas por leer
                if (!is_empty(q)) {
                    ;//log_response(ATTACHMENT(key)->orig.response.request->response);
                    selector_set_interest(key->s, *et->client_fd, OP_NOOP);
                    selector_set_interest(key->s, *et->origin_fd, OP_READ);
                    ret = RESPONSE;
                } else {
                    ;//log_response(ATTACHMENT(key)->orig.response.request->response);
                    selector_set_interest(key->s, *et->origin_fd, OP_NOOP);
                    selector_set_interest(key->s, *et->client_fd, OP_READ);
                    ret = REQUEST;
                }

            } else {
                selector_set_interest(key->s, *et->ext_read_fd, OP_READ);
                selector_set_interest(key->s, *et->client_fd, OP_NOOP);
            }
        } else {
            if(!et->error_wr){
                selector_set_interest(key->s, *et->ext_read_fd, OP_READ);
                selector_set_interest(key->s, *et->client_fd, OP_NOOP);
            }
        }
        ;//metricas->transferred_bytes += n;
    } else if (n == -1){
        ret = ERROR;
    }

    return ret;
}

static void external_transformation_close(struct selector_key * key) {
    struct external_transformation * et  = &ATTACHMENT(key)->et;

    selector_unregister_fd(key->s, *et->ext_read_fd);
    close(*et->ext_read_fd);

    selector_unregister_fd(key->s, *et->ext_write_fd);
    close(*et->ext_write_fd);
}

/* Definición de handlers para cada estado */
static const struct state_definition client_states[] = {
        {
                .state            = ORIGIN_SERVER_RESOLUTION,
                .on_write_ready   = origin_server_resolution,
                .on_block_ready   = origin_server_resolution_done,
        },{
                .state            = CONNECTING_TO_OS,
                .on_write_ready   = connecting,
        },{
                .state            = WELCOME_READ,
                .on_arrival       = welcome_init,
                .on_read_ready    = welcome_read,
        },{
                .state            = WELCOME_WRITE,
                .on_write_ready   = welcome_write,
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
        },{
                .state            = RESPONSE,
                .on_arrival       = response_init,
                .on_read_ready    = response_read,
                .on_write_ready   = response_write,
                .on_departure     = response_close,
        },{
                .state            = EXTERNAL_TRANSFORMATION,
                .on_arrival       = external_transformation_init,
                .on_read_ready    = external_transformation_read,
                .on_write_ready   = external_transformation_write,
                .on_departure     = external_transformation_close,
        },{
                .state            = DONE,

        },{
                .state            = ERROR,
        }
};

static const struct state_definition * pop3_describe_states(void) {
    return client_states;
}