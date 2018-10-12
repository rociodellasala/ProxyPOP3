#ifndef PROXYPOP3_MAIN_H
#define PROXYPOP3_MAIN_H

#define MAXIMUM_MUA_CONNECTIONS 20

typedef int file_descriptor;

int new_socket(int, int, struct sockaddr_in *);

#endif //PROXYPOP3_MAIN_H
