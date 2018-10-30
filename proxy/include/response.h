#ifndef PROXYPOP3_RESPONSE_H
#define PROXYPOP3_RESPONSE_H

enum pop3_response_status {
    OK,
    ERR,
};

enum pop3_response_status parse_response(const char *);

#endif //PROXYPOP3_RESPONSE_H
