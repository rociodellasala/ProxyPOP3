#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>

#include "include/buffer.h"
#include "include/logs.h"

#define N(x) (sizeof(x)/sizeof((x)[0]))

char * print_connection_status(const struct sockaddr * addr) {
    char *  buff        = malloc(100);
    char    buffsize    = 100;

    if (addr == 0) {
        strncpy(buff, "null", buffsize);
        return buff;
    }

    in_port_t   port;
    void *      p       = 0x00;
    bool        handled = false;

    switch (addr->sa_family) {
        case AF_INET:
            p       = &((struct sockaddr_in *) addr)->sin_addr;
            port    =  ((struct sockaddr_in *) addr)->sin_port;
            handled = true;
            break;
        case AF_INET6:
            p       = &((struct sockaddr_in6 *) addr)->sin6_addr;
            port    =  ((struct sockaddr_in6 *) addr)->sin6_port;
            handled = true;
            break;
        default:
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

    if(handled) {
        snprintf(buff + len, buffsize - len, "%d", ntohs(port));
    }

    buff[buffsize - 1] = 0;
    return buff;
}


void log_connection(bool opened, const struct sockaddr * clientaddr, char * message) {
    char        time_buffer[TIME]   = {0};
    unsigned    n                   = N(time_buffer);

    time_t now = 0;
    time(&now);
    strftime(time_buffer, n, "%FT %TZ", gmtime(&now));
    char * connection_info  = print_connection_status(clientaddr);

    if(opened){
        fprintf(stdout, ANSI_GREEN "[%s]:" ANSI_RESET " %s - %s\n", time_buffer, message, connection_info);
    } else {
        fprintf(stdout, ANSI_RED "[%s]:" ANSI_RESET " %s - %s\n", time_buffer, message, connection_info);
    }

    free(connection_info);
}


void log_request(bool opened, char * cmd, char * args, char * message) {
    char        time_buffer[TIME]   = {0};
    unsigned    n                   = N(time_buffer);
    time_t now = 0;
    time(&now);
    strftime(time_buffer, n, "%FT %TZ", gmtime(&now));
    char * request_info;

    if (args != NULL) {
        if (strcmp(cmd,"pass") == 0){
            request_info = malloc((strlen(cmd) + strlen(args) + 25) * sizeof(char *));
            int i = strlen(args);
            int j = 0;
            char * encryped_pass = malloc(i * sizeof(char *));
            while (i > 0) {
                encryped_pass[j++] = '*';
                i--;
            }
            sprintf(request_info, "cmd %s - args %s\n", cmd, encryped_pass);
            free(encryped_pass);
        } else {
            request_info = malloc((strlen(cmd) + strlen(args) + 25) * sizeof(char *));
            sprintf(request_info, "cmd %s - args %s\n", cmd, args);
        }
    } else {
        request_info = malloc((strlen(cmd) + 25) * sizeof(char *));
        sprintf(request_info, "cmd %s - args are null\n", cmd);
    }

    if(opened){
        fprintf(stdout, ANSI_GREEN "[%s]:" ANSI_RESET " %s - %s", time_buffer, message, request_info);
    } else {
        fprintf(stdout, ANSI_RED "[%s]:" ANSI_RESET " %s - %s", time_buffer, message, request_info);
    }

    free(request_info);
}

void log_response(bool opened, char * cmd, char * status, char * message) {
    char        time_buffer[TIME]   = {0};
    unsigned    n                   = N(time_buffer);

    time_t now = 0;
    time(&now);
    strftime(time_buffer, n, "%FT %TZ", gmtime(&now));

    char * request_info = malloc((strlen(cmd) + strlen(status) + 20) * sizeof(char *));

    sprintf(request_info, "cmd: %s - status:%s\n", cmd, status);

    if(opened){
        fprintf(stdout, ANSI_GREEN "[%s]:" ANSI_RESET " %s - %s", time_buffer, message, request_info);
    } else {
        fprintf(stdout, ANSI_RED "[%s]:" ANSI_RESET " %s - %s", time_buffer, message, request_info);
    }

    free(request_info);
}


void log_external_transformation(bool opened, char * message) {
    char time_buffer[50] = {0};
    unsigned    n           = N(time_buffer);

    time_t now = 0;
    time(&now);
    strftime(time_buffer, n, "%FT %TZ", gmtime(&now));

    if(opened){
        fprintf(stdout, ANSI_GREEN "[%s]:" ANSI_RESET " %s\n", time_buffer, message);
    } else {
        fprintf(stdout, ANSI_RED "[%s]:" ANSI_RESET " %s\n", time_buffer, message);
    }

}

void log_origin_server_resolution(bool opened, char * message) {
    char time_buffer[50] = {0};
    unsigned    n           = N(time_buffer);

    time_t now = 0;
    time(&now);
    strftime(time_buffer, n, "%FT %TZ", gmtime(&now));

    if(opened){
        fprintf(stdout, ANSI_GREEN "[%s]:" ANSI_RESET " %s\n", time_buffer, message);
    } else {
        fprintf(stdout, ANSI_RED "[%s]:" ANSI_RESET " %s\n", time_buffer, message);
    }
}
