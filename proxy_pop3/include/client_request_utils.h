#ifndef PROXYPOP3_CLIENT_REQUEST_UTILS_H
#define PROXYPOP3_CLIENT_REQUEST_UTILS_H

#include "pop3_session.h"
#include "client_request.h"
#include "utils.h"
#include "buffer.h"
#include "client_parser_request.h"

void send_error_request(request_state, file_descriptor);

struct pop3_request * request_to_buffer(buffer *, bool, struct pop3_request *, struct msg_queue *);

#endif //PROXYPOP3_CLIENT_REQUEST_UTILS_H
