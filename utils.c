#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <stdlib.h>

#include "include/utils.h"
#include "include/request.h"
#include "include/request_parser.h"

#define N(x) (sizeof(x)/sizeof((x)[0]))

/* TODO: Creo que esto hay que sacarlo */
extern const char * sockaddr_to_human(char * buff, const size_t buffsize, const struct sockaddr * addr) {
    if (addr == 0) {
        strncpy(buff, "null", buffsize);
        return buff;
    }

    in_port_t port;
    void *p = 0x00;
    bool handled = false;

    switch(addr->sa_family) {
        case AF_INET:
            p       = &((struct sockaddr_in *) addr)->sin_addr;
            port    = ((struct sockaddr_in *) addr)->sin_port;
            handled = true;
            break;
        case AF_INET6:
            p       = &((struct sockaddr_in6 *) addr)->sin6_addr;
            port    = ((struct sockaddr_in6 *) addr)->sin6_port;
            handled = true;
            break;
    }

    if (handled) {
        if (inet_ntop(addr->sa_family, p,  buff, buffsize) == 0) {
            strncpy(buff, "unknown ip", buffsize);
            buff[buffsize - 1] = 0;
        }
    } else {
        strncpy(buff, "unknown", buffsize);
    }

    strncat(buff, ":", buffsize);
    buff[buffsize - 1] = 0;
    const size_t len = strlen(buff);

    if (handled) {
        snprintf(buff + len, buffsize - len, "%d", ntohs(port));
    }

    buff[buffsize - 1] = 0;

    return buff;
}

void print_connection_status(const char * msg, struct sockaddr_storage addr, file_descriptor fd) {
    char hoststr[NI_MAXHOST];
    char portstr[NI_MAXSERV];

    getnameinfo((struct sockaddr *) &addr,
                sizeof(addr), hoststr, sizeof(hoststr), portstr, sizeof(portstr),
                NI_NUMERICHOST | NI_NUMERICSERV);

    printf("%s: \n\t - File descriptor: %d \n\t - IP: %s \n\t - Port: %s \n", msg, fd, hoststr, portstr);
}


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


char * append_cmd(char * dest, char * msg, const char * parse_cmd) {
    char cmd[CMD_SIZE];
    dest = malloc((strlen(msg) + CMD_SIZE + 2) * sizeof(char *));
    strcpy(cmd,parse_cmd);
    strcpy(dest, msg);
    strcat(dest, cmd);
    strcat(dest, "\r\n");
    return dest;
}