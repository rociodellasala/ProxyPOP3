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

struct external_transformation {
    enum et_status              status;

    buffer *                    read_buffer;
    buffer *                    write_buffer;
    buffer *                    ext_rb;
    buffer *                    ext_wb;

    file_descriptor *           client_fd;
    file_descriptor *           origin_fd;
    file_descriptor *           ext_read_fd;
    file_descriptor *           ext_write_fd;

    struct parser *             parser_read;
    struct parser *             parser_write;

    bool                        finish_wr;
    bool                        finish_rd;

    bool                        error_wr;
    bool                        error_rd;

    bool                        did_write;
    bool                        write_error;

    size_t                      send_bytes_write;
    size_t                      send_bytes_read;
};

void ext_write(struct selector_key *);
void ext_read(struct selector_key *);
void ext_close(struct selector_key *);

static const struct fd_handler ext_handler = {
        .handle_read   = ext_read,
        .handle_write  = ext_write,
        .handle_close  = ext_close,
        .handle_block  = NULL,
};

enum et_status start_external_transformation(struct selector_key *, struct pop3_session *);

bool parse_mail(buffer *, struct parser *, size_t *);

bool finished_et(struct external_transformation *);

#endif //PROXYPOP3_E_TRANSFORMATION_H
