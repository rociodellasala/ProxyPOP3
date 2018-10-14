#ifndef PROXYPOP3_MAIN_H
#define PROXYPOP3_MAIN_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "optionsParser.h"

#define BACKLOG 10


/* Typedefs */
typedef int file_descriptor;

/* Functions */
void initialize_sockets(options);
file_descriptor new_socket(int, int, struct sockaddr_in *);

#endif //PROXYPOP3_MAIN_H
