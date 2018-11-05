#ifndef PROXYPOP3_CLIENT_REQUEST_UTILS_H
#define PROXYPOP3_CLIENT_REQUEST_UTILS_H

#include "pop3nio.h"
#include "pop3_session.h"
#include "client_request.h"
#include "utils.h"
#include "buffer.h"
#include "client_parser_request.h"

struct pop3_request * request_to_buffer(buffer *, bool, struct pop3_request *, struct msg_queue *);

enum pop3_state process_request(struct selector_key * key, struct request_st * request);

#endif //PROXYPOP3_CLIENT_REQUEST_UTILS_H
