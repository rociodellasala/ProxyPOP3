
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "include/buffer.h"
#include "include/pop3_multi.h"
#include "include/input_parser.h"
#include "include/selector.h"
#include "include/pop3nio.h"

void ext_read(struct selector_key * key) {
    struct external_transformation *et  = &ATTACHMENT(key)->et;
    puts("ext_read");
    buffer  *b                          = et->ext_rb;
    uint8_t *ptr;
    size_t   count;
    ssize_t  n;

    ptr = buffer_write_ptr(b, &count);
    n   = read(*et->ext_read_fd, ptr, count);

    if (n < 0){
        selector_unregister_fd(key->s, key->fd);
        et->error_wr = true;
        selector_set_interest(key->s, *et->client_fd, OP_WRITE);
    } else if (n >= 0){
        buffer_write_adv(b, n);
        if (parse_mail(b, et->parser_write, &et->send_bytes_write)){
            //log_response(ATTACHMENT(key)->orig.response.request->response);
            selector_unregister_fd(key->s, key->fd);
        }else{
            selector_set_interest(key->s, *et->ext_read_fd, OP_NOOP);
            if (n == 0){
                selector_unregister_fd(key->s, key->fd);
                et->error_wr = true;
            }
        }
        selector_set_interest(key->s, *et->client_fd, OP_WRITE);
    }
}

void ext_write(struct selector_key * key) {
    struct external_transformation *et  = &ATTACHMENT(key)->et;
    puts("ext_write");
    buffer  *b                          = et->ext_wb;
    uint8_t *ptr;
    size_t   count;
    ssize_t  n;

    ptr = buffer_read_ptr(b, &count);
    size_t bytes_sent = count;
    if (et->send_bytes_read != 0){
        bytes_sent = et->send_bytes_read;
    }
    n   = write(*et->ext_write_fd, ptr, bytes_sent);

    if (n > 0) {
        if (et->send_bytes_read != 0)
            et->send_bytes_read -= n;
        buffer_read_adv(b, n);
        if (et->finish_rd && et->send_bytes_read == 0){
            selector_unregister_fd(key->s, key->fd);
        }else{
            selector_set_interest(key->s, *et->ext_write_fd, OP_NOOP);
            selector_set_interest(key->s, *et->origin_fd, OP_READ);
        }
    }else if(n == -1){
        et->status = et_status_err;
        if (et->send_bytes_read == 0)
            buffer_reset(b);
        else
            buffer_read_adv(b, et->send_bytes_read);
        selector_unregister_fd(key->s, key->fd);
        selector_set_interest(key->s, *et->origin_fd, OP_READ);
        et->error_rd = true;
    }
}

void ext_close(struct selector_key * key) {
    close(key->fd);
}



/**
 * Return true if the parser finished reading the mail.
 */
bool parse_mail(buffer * b, struct parser * p, size_t * send_bytes) {
    size_t i = 0;
    size_t count;
    uint8_t *rp = buffer_read_ptr(b, &count);
    uint8_t *wp = buffer_write_ptr(b, &count);
    while (buffer_can_read(b)) {
        i++;
        uint8_t c = buffer_read(b);
        const struct parser_event *e = parser_feed(p, c);
        if (e->type == POP3_MULTI_FIN){
            *send_bytes = i;
            b->read  = rp;
            b->write = wp;
            return true;
        }
    }
    b->read  = rp;
    b->write = wp;
    *send_bytes = 0;
    return false;
}

/**
 * When finished, change to request
 */
bool finished_et(struct external_transformation *et) {
    if(et->finish_rd && et->finish_wr){
        return true;
    }else if(et->finish_rd && et->error_wr) {
        return true;
    }
    return false;
}

enum et_status start_external_transformation(struct selector_key * key, struct pop3_session * session) {

    char * censored_medias_typed = "text/plain";

    size_t size = 14 + strlen(censored_medias_typed) + 13 + strlen(parameters->replacement_msg) + 23 +
                  strlen("1.0") + 17 + strlen(session->user) + 15 +
                  strlen(parameters->origin_server) + 2 +
                  strlen((const char *) parameters->filter_command->program_name) + 2;
    char * env_cat = malloc(size);

    sprintf(env_cat, "FILTER_MEDIAS=%s FILTER_MSG=\"%s\" "
                     "POP3_FILTER_VERSION=\"%s\" POP3_USERNAME=\"%s\" POP3_SERVER=\"%s\" %s ",
            censored_medias_typed, parameters->replacement_msg, "1.0", session->user,
            parameters->origin_server, (char *) parameters->filter_command->program_name);

    //free(medias);

    pid_t pid;
    char * args[4];
    args[0] = "bash";
    args[1] = "-c";
    args[2] = env_cat;
    args[3] = NULL;


    int fd_read[2];
    int fd_write[2];

    int r1 = pipe(fd_read);
    int r2 = pipe(fd_write);

    if (r1 < 0 || r2 < 0)
        return et_status_err;

    if ((pid = fork()) == -1)
        perror("fork error");
    else if (pid == 0) {
        dup2(fd_write[0], STDIN_FILENO);
        dup2(fd_read[1], STDOUT_FILENO);

        close(fd_write[1]);
        close(fd_read[0]);

        FILE * f = freopen(parameters->error_file, "a+", stderr);
        if (f == NULL)
            exit(-1);

        int value = execve("/bin/bash", args, NULL);
        perror("execve");
        if (value == -1){
            fprintf(stderr, "Error\n");
        }
    }else{
        close(fd_write[0]);
        close(fd_read[1]);
        free(env_cat);
        struct pop3 * data = ATTACHMENT(key);
        if (selector_register(key->s, fd_read[0], &ext_handler, OP_READ, data) == 0 &&
            selector_fd_set_nio(fd_read[0]) == 0){
            data->extern_read_fd = fd_read[0];
        }else{
            close(fd_read[0]);
            close(fd_write[1]);
            return et_status_err;
        } // read from.
        if (selector_register(key->s, fd_write[1], &ext_handler, OP_WRITE, data) == 0 &&
            selector_fd_set_nio(fd_write[1]) == 0){
            data->extern_write_fd = fd_write[1];
        }else{
            selector_unregister_fd(key->s, fd_write[1]);
            close(fd_read[0]);
            close(fd_write[1]);
            return et_status_err;
        } // write to.

        return et_status_ok;
    }
    return et_status_err;
}

