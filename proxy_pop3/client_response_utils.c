#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "include/utils.h"
#include "include/pop3nio.h"

enum pop3_state process_response(struct selector_key * key, struct response_st * response_st) {
    enum pop3_state stm_next_status;

    switch (response_st->request->cmd->id) {
        case quit:
            selector_set_interest_key(key, OP_NOOP);
            ATTACHMENT(key)->session.state = POP3_UPDATE;
            return DONE;
        case user:
            ATTACHMENT(key)->session.user_name = response_st->request->args;
            break;
        case pass:
            if (response_st->request->response->status == response_status_ok) {
                ATTACHMENT(key)->session.state = POP3_TRANSACTION;
            }
            break;
        default:
            break;
    }

    struct msg_queue * queue = ATTACHMENT(key)->session.request_queue;

    if (!is_empty(queue)) {
        if (ATTACHMENT(key)->session.pipelining) {
            struct pop3_request * request = dequeue(queue);

            if (request == NULL) {
                fprintf(stderr, "Request is NULL");
                abort();
            }

            response_st->request                  = request;
            response_st->response_parser.request  = request;
            response_parser_init(&response_st->response_parser);

            selector_status ss = SELECTOR_SUCCESS;
            ss |= selector_set_interest_key(key, OP_NOOP);
            ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_READ);

            stm_next_status= ss == SELECTOR_SUCCESS ? RESPONSE : ERROR;
        } else {
            selector_status ss = SELECTOR_SUCCESS;
            ss |= selector_set_interest_key(key, OP_NOOP);
            ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_WRITE);

            stm_next_status= ss == SELECTOR_SUCCESS ? REQUEST : ERROR;
        }

    } else {
        selector_status ss = SELECTOR_SUCCESS;
        ss |= selector_set_interest_key(key, OP_READ);
        ss |= selector_set_interest(key->s, ATTACHMENT(key)->origin_fd, OP_NOOP);

        stm_next_status = ss == SELECTOR_SUCCESS ? REQUEST : ERROR;
    }


    return stm_next_status;
}