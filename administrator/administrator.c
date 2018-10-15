#include "include/administrator.h"

/* UTIL VER management.c del lado del proxy */
int main(int argc, char ** argv) {
    int i;
    options opt;
    file_descriptor socket;

    if (parse_input(argc,argv) < 0) {
        return -1;
    }

    opt = initialize_values(opt);
    if (argc > 2) {
        opt = set_options_values(opt, argc, argv);
    }

    socket = initialize_sctp_socket(opt);
    communicate_with_proxy(socket);
}

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
        if (fgets(send_buffer, MAX_BUFFER, stdin) == NULL) {
            close(socket);
            exit(-1);
        }

        if (send(socket, send_buffer, strlen(send_buffer), 0) < 0) {
            perror("An error ocurred trying to send messsage!");
            close(socket);
            exit(0);
        }

        /* To each command sent the proxy sends a reply
         * */
        ret = recv(socket, recv_buffer, MAX_BUFFER, 0);

        if (ret <= 0) {
            perror("An error ocurred trying to recieve messsage!");
            close(socket);
            exit(0);
        }

        recv_buffer[ret] = '\0';
        printf("%s", recv_buffer);

        if (strcmp(recv_buffer,"+OK: Goodbye\n") == 0) {
            close(socket);
            exit(0);
        }
    }
}

file_descriptor initialize_sctp_socket(options opt) {
    /* Ojo tanto aca como en los sockets del filter hay que cambiar AF_INET
     * por uno que soporte tmb ipv6 y bla */
    int ret;
    file_descriptor configuration_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

    if (configuration_socket < 0) {
        fprintf(stderr, "Unable to create socket");
        perror("");
        exit(EXIT_FAILURE);
    }

    /* Construct the server address structure */
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family   = AF_INET;
    serverAddr.sin_port     = htons((uint16_t) opt.management_port);

    /* Establish the connection to the server */
    if (connect(configuration_socket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection to origin server failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Success! Connected to server\n");
    }

    return configuration_socket;
}


