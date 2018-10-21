#include <arpa/inet.h>
#include <stdint-gcc.h>
#include "include/administrator.h"

int main(int argc, char ** argv) {
    int i;
    options opt;
    file_descriptor socket;

    char recv_buffer[MAX_BUFFER] = {0};
    
    if (parse_input(argc,argv) < 0) {
        return -1;
    }

    opt = initialize_values(opt);
    if (argc > 2) {
        opt = set_options_values(opt, argc, argv);
    } else {
        printf("Default ip and address values: \n - [ip] : 127.0.0.1 \n - [port] : 9090 \n");
    }

    socket = initialize_sctp_socket(opt);

    /* Receives the greeting message from proxy */
    recv(socket, recv_buffer, MAX_BUFFER, 0);
    printf("\nMessage from proxy: %s", recv_buffer);
    
    communicate_with_proxy(socket);
}

file_descriptor initialize_sctp_socket(options opt) {
    /* Ojo tanto aca como en los sockets del filter hay que cambiar AF_INET
     * por uno que soporte tmb ipv6 y bla */
    int rtnVal;
    file_descriptor configuration_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

    if (configuration_socket < 0) {
        fprintf(stderr, "Unable to create socket");
        perror("");
        exit(EXIT_FAILURE);
    }

    /* Construct the proxy address structure */
    struct sockaddr_in proxyAddr;
    memset(&proxyAddr, 0, sizeof(proxyAddr));
    proxyAddr.sin_family    = AF_INET;
    rtnVal                  = inet_pton(AF_INET, opt.management_address, &proxyAddr.sin_addr.s_addr);

    if (rtnVal == 0) {
        printf("inet_pton() failed\n");
    } else if (rtnVal < 0) {
        printf("inet_pton() failed\n");
    }
    proxyAddr.sin_port     = htons((uint16_t) opt.management_port);

    /* Establish the connection to proxy */
    if (connect(configuration_socket, (struct sockaddr *) &proxyAddr, sizeof(proxyAddr)) < 0) {
        perror("Connection to POP3 proxy failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Success! Connected to POP3 proxy\n");
    }

    return configuration_socket;
}