#include <sys/socket.h>
#include <unistd.h>
#include <memory.h>
#include <malloc.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netinet/sctp.h>
#include <errno.h>

#include "include/selector.h"
#include "include/input_parser.h"
#include "include/utils.h"
#include "include/admin.h"
#include "include/parse_helpers.h"
#include "include/request_admin.h"
#include "include/deserializer.h"
#include "include/admin_actions.h"

#define ATTACHMENT(key) ((struct admin *)(key)->data)
#define N(x) (sizeof(x)/sizeof((x)[0]))

options parameters;

void admin_read(struct selector_key * key);
void admin_write(struct selector_key * key);
void admin_block(struct selector_key * key);
void admin_close(struct selector_key * key);

static const struct fd_handler admin_handler = {
        .handle_read   = admin_read,
        .handle_write  = admin_write,
        .handle_close  = admin_close,
        .handle_block  = admin_block,
};

struct admin * admin_new(const int client_fd) {
    struct admin * ret;
    ret = malloc(sizeof(*ret));

    if (ret == NULL){
        return ret;
    }

    ret->client_fd     = client_fd;
    
        /* ME PA Q HAY Q BORRAR ESTO PORQUE NO ESTOY USANDO LOS BUFFERS DE ADMIN */
    buffer_init(&ret->buffer_write, N(ret->raw_buffer_write),ret->raw_buffer_write);
    buffer_init(&ret->buffer_read , N(ret->raw_buffer_read) ,ret->raw_buffer_read);

    ret->status = ST_AUTH;

    return ret;
}

void admin_destroy(struct admin * corpse) {
    free(corpse);
}

// handler functions

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

    print_connection_status("Connection established", client_addr);
    printf("[ADMIN]: File descriptor: %d\n", client);
    state = admin_new(client);

    if(state == NULL) {
        // TODO: sin un estado, nos es imposible manejaro.
        // tal vez deberiamos apagar accept() hasta que detectemos
        // que se liberó alguna conexión.
        goto fail;
    }

    memcpy(&state->client_addr, &client_addr, client_addr_len);
    state->client_addr_len = client_addr_len;

    if(SELECTOR_SUCCESS != selector_register(key->s, client, &admin_handler,
                                             OP_READ, state)) {
        goto fail;
    }

    char * message = "Welcome to POP3 Proxy Management Server\n";
    /* TODO: No deberiamos enviarlo como un datagrama ? */
    sctp_sendmsg(client, message, strlen(message), NULL, 0, 0, 0, 0, 0, 0);
    return ;

    fail:
    if(client != -1) {
        close(client);
    }

    admin_destroy(state);
}

void admin_read(struct selector_key * key) {
    int status;
    int rd_sz;
    file_descriptor admin_fd = key->fd;
    request * request = malloc(sizeof(request));
    unsigned char buffer[40];

    rd_sz = sctp_recvmsg(key->fd, buffer, 40, NULL,0,0,0);

    if (rd_sz <= 0) {
        printf("%s\n", strerror(errno));
    }

    deserialize_request(buffer, request);

    switch(request->cmd){
        case AUTH:
            printf("PIDIENDO EL COMANDO AUTH\n");
            check_password(request, &status);
            send_response_without_data(admin_fd, status);
            break;
        case SET_TRANSF:
            printf("PIDIENDO EL COMANDO SET_TRANSF\n");
            send_response_without_data(admin_fd, status);
            break;
        case GET_TRANSF:
            printf("PIDIENDO EL COMANDO GET_TRANSF\n");
            send_response_without_data(admin_fd, status);
            break;
        case SWITCH_TRANSF:
            printf("PIDIENDO EL COMANDO SWITCH_TRANSF\n");
            send_response_without_data(admin_fd, status);
            break;
        case GET_METRIC:
            printf("PIDIENDO EL COMANDO GET_METRIC\n");
            check_password(request, &status);
            send_response_without_data(admin_fd, status);
            break;
        case GET_MIME:
            printf("PIDIENDO EL COMANDO GET_MIME\n");
            status = 1;
            send_response_with_data(parameters->filtered_media_types, admin_fd, status);
            break;
        case ALLOW_MIME:
            printf("PIDIENDO EL COMANDO ALLOW_MIME\n");
            //allow_mime(request, &status, opt->filtered_media_types);
            send_response_without_data(admin_fd, status);
            break;
        case FORBID_MIME:
            printf("PIDIENDO EL COMANDO FORBID_MIME\n");
            //forbid_mime(request, &status, opt->filtered_media_types);
            send_response_without_data(admin_fd, status);
            break;
        case QUIT:
            printf("PIDIENDO EL COMANDO QUIT\n");
            //liberar_todos_los_recursos_del_admin();
            send_response_without_data(admin_fd, status);
            break;
    }

}

void admin_write(struct selector_key * key){
    selector_set_interest(key->s, key->fd, OP_READ);
}


void admin_block(struct selector_key * key) {

}

void admin_close(struct selector_key * key) {
    struct admin * data = ATTACHMENT(key);
    print_connection_status("Connection disconnected", data->client_addr);
    admin_destroy(data);
}

/* PARSER */

int parse_auth(struct admin * data, char ** cmd);

struct parse_action {
    parse_status status;
    int (* action)(struct admin * data, char ** cmd);
    int args;
};

static struct parse_action auth_action = {
        .status      = ST_AUTH,
        .action      = parse_auth,
        .args        = 1,
};

static struct parse_action * action_list[] = {
        &auth_action,
};

int parse_commands(struct admin * data) {
    char ** cmd;
    struct parse_action * act = action_list[data->status];
    int status = 0;
    int st_err;
    data->argc = 0;

    cmd = sctp_parse_cmd(&data->buffer_read, data, &data->argc, &st_err);

    if (st_err != ERROR_DISCONNECT && act->args != 0 && act->args != data->argc) {
        st_err = ERROR_WRONGARGS;
    }

    switch (st_err) {
        case PARSE_OK:
            status = act->action(data, cmd);
            free_cmd(cmd, act->args);
            break;
        case ERROR_WRONGARGS:
            send_error(data, "wrong command or wrong number of arguments.");
            break;
        case ERROR_MALLOC:
            send_error(data, "server error. Try again later.");
        case ERROR_DISCONNECT:
            printf("ACA 3\n");
            return -1;
        default:
            break;
    }

    return status;
}

int parse_auth(struct admin * data, char ** cmd) {
    printf("ACA EN PARSE AUTH\n");
    if (strcasecmp("AUTH", cmd[0]) == 0) {
        send_ok(data, "hello!");
        data->status = ST_TRANS;
    } else {
        send_error(data, "command not recognized.");
    }

    return 0;
}
