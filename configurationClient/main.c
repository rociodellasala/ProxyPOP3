#include <arpa/inet.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include "include/administrator.h"
#include "include/input_parser.h"

options parameters;



/* todo: VER CASO DONDE SE ROMPA EL PROXY, DESCONECTAR AL ADMIN */

struct addrinfo * dns_resolution(){
    struct addrinfo * list_result = 0;
    char mgmt_buff[10];
    snprintf(mgmt_buff, sizeof(mgmt_buff), "%hu", parameters->management_port);

    struct addrinfo hints = {
            .ai_family    = AF_UNSPEC,    /* Allow IPv4 or IPv6 */
            .ai_socktype  = SOCK_STREAM,  /* Datagram socket */
            .ai_flags     = AI_PASSIVE,   /* For wildcard IP address */
            .ai_protocol  = 0,            /* Any protocol */
            .ai_canonname = NULL,
            .ai_addr      = NULL,
            .ai_next      = NULL,
    };

    if (getaddrinfo(parameters->management_address, mgmt_buff, &hints, &list_result) != 0){
        fprintf(stderr,"Domain Name System (DNS) resolution error\n");
    }

    return list_result;
}

void initialize_sctp_socket() {
    struct sockaddr_in proxyAddr;
    struct addrinfo * managementaddrinfo = dns_resolution();

    socket_fd = socket(managementaddrinfo->ai_family, SOCK_STREAM, IPPROTO_SCTP);

    if (socket_fd < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    /* Establece la conexion al proxy */
    if (connect(socket_fd, managementaddrinfo->ai_addr, managementaddrinfo->ai_addrlen) < 0) {
        perror("Connection to POP3 proxy failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Success! Connected to POP3 proxy\n");
    }

    freeaddrinfo(managementaddrinfo);
}

int main(int argc, char ** argv) {
    int i;
    char recv_buffer[MAX_BUFFER] = {0};

    if (parse_input(argc,argv) < 0) {
        return -1;
    }

    initialize_values();

    if (argc > 2) {
        set_options_values(argc, argv);
    } else {
        printf("Default ip and address values: \n - [ip] : 127.0.0.1 \n - [port] : 9090 \n");
    }

    initialize_sctp_socket();

    /* Recibe el mensaje de bienvenida del proxy */
    sctp_recvmsg(socket_fd, (void *) recv_buffer, MAX_BUFFER, NULL, 0, 0, 0);
    printf("\nMessage from proxy: %s", recv_buffer);

    communicate_with_proxy();

    close(socket_fd);
}