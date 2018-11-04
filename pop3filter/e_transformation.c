
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


char * init_enviroment_variables(struct pop3_session * session){
    char * enviroment_var;

    char * filter_medias        = "FILTER_MEDIAS=";
    char * filter_msg           = "FILTER_MSG=";
    char * pop3_filter_version  = "POP3_FILTER_VERSION=";
    char * pop3_username        = "POP3_USERNAME=";
    char * pop3_server          = "POP3_SERVER=";

    char * censored_medias_typed = "text/plain"; //llamar a funcion

    size_t size =   strlen(filter_medias) + strlen(censored_medias_typed) +
                    strlen(filter_msg) + strlen(parameters->replacement_msg) + 2 +
                    strlen(pop3_filter_version) + strlen(parameters->version) + 2 +
                    strlen(pop3_username) + strlen(session->user_name) + 2 +
                    strlen(pop3_server) + strlen(parameters->origin_server) + 2 +
                    strlen((const char *) parameters->filter_command->program_name) +6 ;

    enviroment_var = malloc(size);

    sprintf(enviroment_var, "%s=%s %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s",
            filter_medias, censored_medias_typed,
            filter_msg, parameters->replacement_msg,
            pop3_filter_version, parameters->version,
            pop3_username, session->user_name,
            pop3_server, parameters->origin_server,
            (char *) parameters->filter_command->program_name);

    //free(censored_medias_typed);
    return enviroment_var;
}

enum et_status add_to_selector(struct selector_key * key, file_descriptor pipeChildToFather[2], file_descriptor pipeFatherToChild[2]) {
    struct pop3 * data = ATTACHMENT(key);

    if (selector_register(key->s, pipeChildToFather[READ], &ext_handler, OP_READ, data) == 0 &&
        selector_fd_set_nio(pipeChildToFather[READ]) == 0) {
        data->extern_read_fd = pipeChildToFather[READ];
    } else {
        close(pipeChildToFather[READ]);
        close(pipeFatherToChild[WRITE]);
        return et_status_err;
    }// read from.

    if (selector_register(key->s, pipeFatherToChild[WRITE], &ext_handler, OP_WRITE, data) == 0 &&
        selector_fd_set_nio(pipeFatherToChild[WRITE]) == 0) {
        data->extern_write_fd = pipeFatherToChild[WRITE];
    } else {
        selector_unregister_fd(key->s, pipeFatherToChild[1]);
        close(pipeChildToFather[READ]);
        close(pipeFatherToChild[WRITE]);
        return et_status_err;
    } // write to.

    return et_status_done;
}

enum et_status start_external_transformation(struct selector_key * key, struct pop3_session * session) {
    char * enviroment_var = init_enviroment_variables(session);

    char argc = 4;
    char * argv[argc];
    argv[0] = "bash";
    argv[1] = "-c";
    argv[2] = enviroment_var;
    argv[3] = NULL;

    pid_t pid;
    file_descriptor pipeChildToFather[2];
    file_descriptor pipeFatherToChild[2];

    if (pipe(pipeChildToFather) < 0 || pipe(pipeFatherToChild) < 0) {
        return et_status_err;
    }

    pid = fork();

    if (pid == -1) {
        perror("fork error");
    } else if (pid == 0) {
        if(dup2(pipeFatherToChild[READ], STDIN_FILENO) == -1) {
            return et_status_err;
        }

        if (dup2(pipeChildToFather[WRITE], STDOUT_FILENO) == -1) {
            return et_status_err;
        }

        close(pipeFatherToChild[WRITE]);
        close(pipeChildToFather[READ]);

        FILE * f = freopen(parameters->error_file, "a+", stderr);

        if (f == NULL) {
            exit(EXIT_FAILURE);
        }

        int value = execve("/bin/bash", argv, NULL);
        perror("execve");

        if (value == -1){
            fprintf(stderr, "Error\n");
        }

    } else {
        close(pipeFatherToChild[READ]);
        close(pipeChildToFather[1]);
        free(enviroment_var);

        if (add_to_selector(key, pipeChildToFather, pipeFatherToChild) == et_status_err) {
            return et_status_err;
        }

        return et_status_ok;
    }

    return et_status_err;
}

