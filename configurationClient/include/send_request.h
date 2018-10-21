#ifndef PROXYPOP3_SEND_REQUEST_H
#define PROXYPOP3_SEND_REQUEST_H

#include "administrator.h"

void send_request_one_param(enum req_cmd, const char *, file_descriptor);
void send_request_without_param(enum req_cmd cmd, file_descriptor socket);
ssize_t send_request(file_descriptor socket, request * request);

#endif //PROXYPOP3_SEND_REQUEST_H
