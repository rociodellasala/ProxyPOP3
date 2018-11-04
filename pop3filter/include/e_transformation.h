#ifndef PROXYPOP3_E_TRANSFORMATION_H
#define PROXYPOP3_E_TRANSFORMATION_H

#include "buffer.h"

enum et_status {
    et_status_ok,
    et_status_err,
    et_status_done,
};

struct external_transformation {
    enum et_status              status;

    buffer                      *rb, *wb;
    buffer                      *ext_rb, *ext_wb;

    int                         *client_fd, *origin_fd;
    int                         *ext_read_fd, *ext_write_fd;

    struct parser               *parser_read;
    struct parser               *parser_write;

    bool                        finish_wr;
    bool                        finish_rd;

    bool                        error_wr;
    bool                        error_rd;

    bool                        did_write;
    bool                        write_error;

    size_t                      send_bytes_write;
    size_t                      send_bytes_read;
};

void ext_write(struct selector_key * key);
void ext_read(struct selector_key * key);
void ext_close(struct selector_key * key);

static const struct fd_handler ext_handler = {
        .handle_read   = ext_read,
        .handle_write  = ext_write,
        .handle_close  = ext_close,
        .handle_block  = NULL,
};

enum et_status open_external_transformation(struct selector_key * key, struct pop3_session * session);

bool parse_mail(buffer * b, struct parser * p, size_t * send_bytes);

bool finished_et(struct external_transformation *et);

#endif //PROXYPOP3_E_TRANSFORMATION_H
