#ifndef PROXYPOP3_UTILS_H
#define PROXYPOP3_UTILS_H

#include "response.h"

typedef int file_descriptor;

void show_menu_authentication();

void show_menu_transaction();

void print_msg(response_status, response);

#endif //PROXYPOP3_UTILS_H
