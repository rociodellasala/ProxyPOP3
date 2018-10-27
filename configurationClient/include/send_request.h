#ifndef PROXYPOP3_SEND_REQUEST_H
#define PROXYPOP3_SEND_REQUEST_H

#include "utils.h"
#include "administrator.h"

void send_request_one_param(const char *, enum cmd);
void send_request_without_param(enum cmd);

#endif //PROXYPOP3_SEND_REQUEST_H
