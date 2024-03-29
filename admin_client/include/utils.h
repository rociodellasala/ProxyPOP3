#ifndef PROXYPOP3_UTILS_H
#define PROXYPOP3_UTILS_H

#include "response.h"

#define VERSION 0X01
#define MAX_PARAM 100
#define MAX_BUFFER 9 + MAX_PARAM
#define NEW_LINE '\n'
#define SPACE ' '

typedef int file_descriptor;

void show_menu_authentication();
void show_menu_transaction();
void print_msg(enum response_status, struct response);

#endif //PROXYPOP3_UTILS_H