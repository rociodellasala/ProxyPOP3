#ifndef PROXYPOP3_SELECTOR_H
#define PROXYPOP3_SELECTOR_H

#define MAXIMUM_MUA_CONNECTIONS 20
#define DATA_BUFFER 1024

typedef int file_descriptor;

void handle_connections(file_descriptor mua_tcp_socket, struct sockaddr_in address);

#endif //PROXYPOP3_SELECTOR_H
