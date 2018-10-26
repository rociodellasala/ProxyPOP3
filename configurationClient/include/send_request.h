#ifndef PROXYPOP3_SEND_REQUEST_H
#define PROXYPOP3_SEND_REQUEST_H

#include "utils.h"
#include "administrator.h"

void send_request_one_param(const char *, enum cmd, file_descriptor);
void send_request_without_param(enum cmd, file_descriptor);

#endif //PROXYPOP3_SEND_REQUEST_H
