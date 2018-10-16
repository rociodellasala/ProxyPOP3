#include <arpa/inet.h>
#include "include/administrator.h"

/* APROVECHAR EL USO DE DATAGRAMAS ???? */
void communicate_with_proxy(file_descriptor socket) {
    int ret;
    int running                  = 1;
    char send_buffer[MAX_BUFFER] = {0};
    char recv_buffer[MAX_BUFFER] = {0};

    /* Receives the greeting message from proxy */
    recv(socket, recv_buffer, MAX_BUFFER, 0);
    printf("%s", recv_buffer);

    /* Once greeting message from proxy is received, we can send username
     * and password. After that, administrator can send commands */
    while (running) {
        /* Ojo, es bloqueante */
        if (fgets(send_buffer, MAX_BUFFER, stdin) == NULL) {
            printf("error");
            close(socket);
            exit(-1);
        }

        send_buffer[strcspn(send_buffer, "\r\n")] = 0;

        if (send(socket, send_buffer, strlen(send_buffer), 0) != strlen(send_buffer)) {
            perror("An error ocurred trying to send messsage!");
            close(socket);
            exit(0);
        }

        /* To each command sent the proxy sends a reply */
        clear_send_buffer(send_buffer);
        clear_recv_buffer(recv_buffer);

        ret = recv(socket, recv_buffer, MAX_BUFFER, 0);

        if (ret <= 0) {
            perror("An error ocurred trying to recieve messsage from proxy!");
            close(socket);
            exit(0);
        }

        recv_buffer[ret] = '\0';
        printf("%s\n", recv_buffer);

        if (strcmp(recv_buffer,"+OK: Goodbye\n") == 0) {
            close(socket);
            exit(0);
        }

    }
}

void clear_recv_buffer(char * buffer) {
    char * aux = buffer;

    while (*aux != DELIM) {
        *aux = 0;
        aux++;
    }

    *aux = 0;
}

void clear_send_buffer(char * buffer) {
    char * aux = buffer;

    while (*aux != 0) {
        *aux = 0;
        aux++;
    }

    *aux = 0;
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
        perror("Connection to proxy failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Success! Connected to proxy\n");
    }

    return configuration_socket;
}


