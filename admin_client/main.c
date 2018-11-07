#include <arpa/inet.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <strings.h>
#include <memory.h>

#include "include/admin.h"
#include "include/input_parser.h"

options clt_parameters;

struct addrinfo * resolution() {
    char                service[5];
    struct addrinfo *   list_result = 0;

    snprintf(service, sizeof(service), "%hu", clt_parameters->server_port);

    struct addrinfo hints = {
            .ai_family    = AF_UNSPEC,
            .ai_socktype  = SOCK_STREAM,
            .ai_flags     = AI_PASSIVE,
            .ai_protocol  = 0,
            .ai_canonname = NULL,
            .ai_addr      = NULL,
            .ai_next      = NULL,
    };

    if (getaddrinfo(clt_parameters->server_address, service, &hints, &list_result) != 0) {
        fprintf(stderr,"Domain Name System (DNS) resolution error\n");
    }

    return list_result;
}

void initialize_sctp_socket() {
    struct addrinfo * managementaddrinfo = resolution();

    socket_fd = socket(managementaddrinfo->ai_family, SOCK_STREAM, IPPROTO_SCTP);

    if (socket_fd < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }
    
    if (connect(socket_fd, managementaddrinfo->ai_addr, managementaddrinfo->ai_addrlen) < 0) {
        perror("Connection to POP3 proxy_pop3 failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Success! Connected to POP3 proxy_pop3\n");
    }

    freeaddrinfo(managementaddrinfo);
}

int main(int argc, char ** argv) {
    char recv_buffer[MAX_BUFFER] = {0};

    if (parse_input(argc,argv) < 0) {
        return -1;
    }

    printf("Server address and port values: \n - [ip] : %s \n - [port] : %d \n",
           clt_parameters->server_address, clt_parameters->server_port);

    initialize_sctp_socket();

    
    sctp_recvmsg(socket_fd, (void *) recv_buffer, MAX_BUFFER, NULL, 0, 0, 0);
    printf("\nMessage from proxy_pop3: %s", recv_buffer);

    communicate_with_proxy();
    close(socket_fd);
    return 0;
}
