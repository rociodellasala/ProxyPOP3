#ifndef PROXYPOP3_UTILS_H
#define PROXYPOP3_UTILS_H

#define SOCKADDR_TO_HUMAN_MIN (INET6_ADDRSTRLEN + 5 + 1)

typedef int file_descriptor;

/* Describe de forma humana un sockaddr */
const char * sockaddr_to_human(char *, size_t, const struct sockaddr *);

/* */
void print_connection_status(const char *, struct sockaddr_storage);

#endif //PROXYPOP3_UTILS_H
