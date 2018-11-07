#ifndef PROXYPOP3_LOGS_H
#define PROXYPOP3_LOGS_H

#include <stdbool.h>
#include "client_request.h"

#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN    "\x1b[32m"
#define ANSI_RESET   "\x1b[0m"

#define TIME 40

void log_connection(bool, const struct sockaddr *, char *);

void log_request(bool, char *, char *, char *);

void log_response(bool, char*, char *, char *);

void log_external_transformation(bool opened, char * message);

void log_origin_server_resolution(bool opened, char * message);

void log_done();

void log_error();

#endif //PROXYPOP3_LOGS_H
