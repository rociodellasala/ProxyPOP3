#ifndef PROXYPOP3_CLIENT_REQUEST_UTILS_H
#define PROXYPOP3_CLIENT_REQUEST_UTILS_H

#include "include/pop3_session.h"
#include "include/client_request.h"
#include "include/utils.h"

enum request_state check_request_against_current_session_status(enum pop3_session_state, struct pop3_request *);

void send_error_request(enum request_state, char *, file_descriptor);

#endif //PROXYPOP3_CLIENT_REQUEST_UTILS_H
