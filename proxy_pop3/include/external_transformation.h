#ifndef PROXYPOP3_E_TRANSFORMATION_H
#define PROXYPOP3_E_TRANSFORMATION_H

#include "buffer.h"

#define READ 0
#define WRITE 1

enum et_status {
    et_status_ok,
    et_status_err,
    et_status_done,
};

/* Estructura principal de la transformacion externa */
struct external_transformation {
    enum et_status              status;

    file_descriptor *           client_fd;
    file_descriptor *           origin_fd;
    file_descriptor *           ext_read_fd;
    file_descriptor *           ext_write_fd;

    size_t                      send_bytes_write;
    size_t                      send_bytes_read;

    buffer *                    read_buffer;
    buffer *                    write_buffer;
    buffer *                    ext_rb;
    buffer *                    ext_wb;

    bool                        finish_write;
    bool                        finish_read;
    bool                        error_write;
    bool                        error_read;
    bool                        did_write;
    bool                        write_error;

    struct parser *             parser_read;
    struct parser *             parser_write;
};

void ext_write(struct selector_key *);
void ext_read(struct selector_key *);
void ext_close(struct selector_key *);

/*
 * Declaración de los handlers para la transformacion externa
 */
static const struct fd_handler ext_handler = {
        .handle_read   = ext_read,
        .handle_write  = ext_write,
        .handle_close  = ext_close,
        .handle_block  = NULL,
};

enum et_status start_external_transformation(struct selector_key *, struct pop3_session *);

bool parse_mail(buffer *, size_t *, struct parser *);

bool finished_et(struct external_transformation *);

#endif //PROXYPOP3_E_TRANSFORMATION_H
