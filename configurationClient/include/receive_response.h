#ifndef PROXYPOP3_RECEIVE_RESPONSE_H
#define PROXYPOP3_RECEIVE_RESPONSE_H

#include "utils.h"
#include "response.h"

ssize_t receive_response(unsigned char *, response *, const file_descriptor);

#endif //PROXYPOP3_RECEIVE_RESPONSE_H
