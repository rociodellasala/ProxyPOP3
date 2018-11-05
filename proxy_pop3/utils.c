#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <stdlib.h>

#include "include/utils.h"
#include "include/client_request.h"
#include "include/client_parser_request.h"

#define N(x) (sizeof(x)/sizeof((x)[0]))

int get_int_len(int value) {
    int l=1;
    while (value>9) {
        l++;
        value /= 10;
    }
    return l;
}

bool compare_strings(const char * str1, const char * str2) {
    while (*str1 && *str2) {
        if (toupper(*str1++) != toupper(*str2++)) {
            return false;
        }
    }

    if (*str1 == '\0' && *str2 == '\0') {
        return true;
    }

    return false;
}

char * to_upper(char * str) {
    char * aux = str;
    while (*aux != 0) {
        *aux = (char)toupper(*aux);
        aux++;
    }

    return str;
}