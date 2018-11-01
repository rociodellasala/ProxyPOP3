#ifndef PROXYPOP3_ADMIN_PARSER_H
#define PROXYPOP3_ADMIN_PARSER_H

#include "utils.h"

/* Request */
void parse_admin_request(struct admin *);

/* Response */
int parse_admin_response(struct admin *);
void send_response_without_data(struct admin *, unsigned char);

#endif //PROXYPOP3_ADMIN_PARSER_H
