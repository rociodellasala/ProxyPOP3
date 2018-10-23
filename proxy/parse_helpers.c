#include <stdlib.h>
#include <memory.h>
#include <sys/socket.h>
#include <errno.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#include "include/admin.h"
#include "include/parse_helpers.h"

#define BLOCK 10
#define POPG_ARGC_BLOCK 5 // Limit of arguments for POPG

/*
 * If args = 0. Accept all commands.
 */
char ** sctp_parse_cmd(buffer *b, struct admin *data, int *args, int *st_err){
    int argc = POPG_ARGC_BLOCK;
    char ** cmd = malloc(argc * sizeof(char *));

    size_t count;
    uint8_t * ptr = buffer_write_ptr(&data->buffer_read, &count);
    ssize_t length;
    size_t current_arg = 0, current_size = 0, current_index = 0;
    struct sctp_sndrcvinfo sndrcvinfo;
    bool error = false;
    bool copying = false;
    bool quote = false;
    int flags;

    while (true) {
        length = sctp_recvmsg(data->client_fd, ptr, count, NULL, 0, &sndrcvinfo, &flags);
        if (length <= 0) {
            if (errno == EWOULDBLOCK) {
                return 0;
            } else {
                *st_err = ERROR_DISCONNECT;
                return NULL;
            }
        }

        if (error) {
            continue;
        } else {
            buffer_write_adv(&data->buffer_read, length);
        }

        char c;

        while (buffer_can_read(b) && !error) {
            c = buffer_read(b);
            if (!copying) {
                if (!isspace(c)) {
                    if (current_arg == argc) {
                        argc += POPG_ARGC_BLOCK;
                        void * tmp = realloc(cmd, argc * sizeof(char *));
                        if (tmp == NULL){
                            free_cmd(cmd, (int) current_arg);
                            error = true;
                            buffer_reset(b);
                            *st_err = ERROR_MALLOC;
                        }
                        cmd = tmp;
                    }
                    copying = true;
                    cmd[current_arg] = malloc(BLOCK * sizeof(char));
                    if (cmd[current_arg] == NULL) {
                        free_cmd(cmd, (int) current_arg);
                        error = true;
                        buffer_reset(b);
                        *st_err = ERROR_MALLOC;
                    }
                    current_size = BLOCK;
                    current_index = 0;
                    (*args)++;
                    if (c != '\'') {
                        cmd[current_arg][current_index++] = c;
                    } else {
                        quote = true;
                    }
                }
            } else {
                if ((isspace(c) && !quote) || (c == '\'' && quote)) {
                    if (current_index == current_size) {
                        void * tmp = realloc(cmd[current_arg], current_size+1);
                        if (tmp == NULL) {
                            free_cmd(cmd, (int) current_arg);
                            error = true;
                            buffer_reset(b);
                            *st_err = ERROR_MALLOC;
                        }
                        cmd[current_arg] = tmp;
                    }
                    cmd[current_arg][current_index] = '\0';
                    copying = false;
                    quote = false;
                    current_arg++;
                } else {
                    if (current_index == current_size) {
                        current_size += BLOCK;
                        void * tmp = realloc(cmd[current_arg], current_size);
                        if (tmp == NULL) {
                            free_cmd(cmd, (int) current_arg);
                            error = true;
                            *st_err = ERROR_MALLOC;
                        }
                        cmd[current_arg] = tmp;
                    }
                    cmd[current_arg][current_index++] = c;
                }
            }
        }

        if ((flags & MSG_EOR) != 0) {
            if (copying) {
                if (current_index == current_size) {
                    cmd[current_arg] = realloc(cmd[current_arg], current_size+1);
                    if (cmd[current_arg] == NULL) {
                        free_cmd(cmd, (int) current_arg);
                        buffer_reset(b);
                        *st_err = ERROR_MALLOC;
                        error = true;
                    }
                }
                cmd[current_arg][current_index] = '\0';
            }
            if (error) {
                return NULL;
            }
            *st_err = PARSE_OK;
            return cmd;
        }
    }

}
void free_cmd(char ** cmd, int args) {
    int i;

    for (i = 0; i < args; i++) {
        free(cmd[i]);
    }

    free(cmd);
}

void send_error(struct admin * data, const char * text) {
    char * msg = malloc(strlen("-ERR: ") + strlen(text) + 2);

    strcpy(msg, "-ERR: ");
    strcat(msg, text);
    strcat(msg, "\n");

    send(data->client_fd, msg, strlen(msg), 0);

    free(msg);
}

void send_ok(struct admin * data, const char * text) {
    char * msg = malloc(strlen("+OK: ") + strlen(text) + 2);

    strcpy(msg, "+OK: ");
    strcat(msg, text);
    strcat(msg, "\n");

    send(data->client_fd, msg, strlen(msg), 0);
}