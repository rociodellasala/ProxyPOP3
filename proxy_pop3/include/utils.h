#ifndef PROXYPOP3_UTILS_H
#define PROXYPOP3_UTILS_H

#include <stdbool.h>

#define VERSION 0X01
#define MAX_ADMIN_DATA 100
#define MAX_ADMIN_BUFFER 9 + MAX_ADMIN_DATA /* 3 version (uchar), 3 cmd (uchar), 3 length y 100 max data (definido por nosotros) */

typedef int file_descriptor;

int get_int_len(int);

bool compare_strings(const char *, const char *);


char * to_upper(char *);

#endif //PROXYPOP3_UTILS_H
