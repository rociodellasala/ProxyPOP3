#ifndef PROXYPOP3_CONFIGURATIONCLIENT_H
#define PROXYPOP3_CONFIGURATIONCLIENT_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "input_parser.h"

#define MAX_BUFFER 1024

typedef int file_descriptor;

/* Functions */
void communicate_with_proxy(file_descriptor);
file_descriptor initialize_sctp_socket(options);

#endif //PROXYPOP3_CONFIGURATIONCLIENT_H
