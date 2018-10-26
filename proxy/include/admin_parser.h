<<<<<<< HEAD
//
// Created by rocio on 24/10/18.
//

#ifndef PROXYPOP3_ADMIN_PARSER_H
#define PROXYPOP3_ADMIN_PARSER_H

=======
#ifndef PROXYPOP3_ADMIN_PARSER_H
#define PROXYPOP3_ADMIN_PARSER_H

/* Request */

int parse_admin_request(struct admin *);

/* Response */

void parse_admin_response(struct admin *);
void send_response_without_data(file_descriptor, unsigned char);

>>>>>>> f63ebf25f7c99555cb02535cd0d41dc4fef5c2bc
#endif //PROXYPOP3_ADMIN_PARSER_H
