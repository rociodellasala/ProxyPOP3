#ifndef PROXYPOP3_SELECTOR_H
#define PROXYPOP3_SELECTOR_H

#include "optionsParser.h"


#define BUFFER_SIZE 1024

typedef int file_descriptor;

void handle_connections(options opt, file_descriptor mua_tcp_socket, struct sockaddr_in address);

#endif //PROXYPOP3_SELECTOR_H
