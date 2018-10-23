#ifndef PROXYPOP3_RESPONSE_H_
#define PROXYPOP3_RESPONSE_H_

enum pop3_response_status {
    OK,
    ERR,
};

enum pop3_response_status parse_response(const char *);

#endif //PROXYPOP3_RESPONSE_H_