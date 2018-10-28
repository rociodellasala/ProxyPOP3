#ifndef PROXYPOP3_UTILS_H
#define PROXYPOP3_UTILS_H

#define SOCKADDR_TO_HUMAN_MIN (INET6_ADDRSTRLEN + 5 + 1)
#define MAX_ADMIN_DATA 100
#define MAX_ADMIN_BUFFER 9 + MAX_ADMIN_DATA /* 3 version (uchar), 3 cmd (uchar), 3 length y 100 max data (definido por nosotros) */

typedef int file_descriptor;

/* Describe de forma humana un sockaddr */
const char * sockaddr_to_human(char *, size_t, const struct sockaddr *);

void print_connection_status(const char *, struct sockaddr_storage);

int get_int_len(int);

#endif //PROXYPOP3_UTILS_H
