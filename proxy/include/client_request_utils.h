#ifndef PROXYPOP3_CLIENT_REQUEST_UTILS_H
#define PROXYPOP3_CLIENT_REQUEST_UTILS_H

#include "pop3_session.h"
#include "client_request.h"
#include "utils.h"
#include "buffer.h"

enum request_state check_request_against_current_session_status(enum pop3_session_state, struct pop3_request *);

void send_error_request(enum request_state, char *, file_descriptor);

int request_to_buffer(buffer * buffer, bool pipelining, struct pop3_request * pop3_request, struct queue * queue);

#endif //PROXYPOP3_CLIENT_REQUEST_UTILS_H
