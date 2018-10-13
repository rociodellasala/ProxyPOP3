#ifndef PROXYPOP3_MAIN_H
#define PROXYPOP3_MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "selector.h"

#define MAXIMUM_MUA_CONNECTIONS 20

/* Typedefs */
typedef int file_descriptor;

/* Functions */
file_descriptor new_socket(int, int, struct sockaddr_in *);

void initialize_sockets(options);

#endif //PROXYPOP3_MAIN_H
