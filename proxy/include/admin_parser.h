#ifndef PROXYPOP3_ADMIN_PARSER_H
#define PROXYPOP3_ADMIN_PARSER_H

#include "utils.h"

/* Request */

int parse_admin_request(struct admin *);

/* Response */

void parse_admin_response(struct admin *);
void send_response_without_data(file_descriptor, unsigned char);

#endif //PROXYPOP3_ADMIN_PARSER_H
