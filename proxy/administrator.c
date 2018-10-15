#include "include/administrator.h"
#include "include/main.h"

admin accept_admin_connection(proxy_pop3 * proxy){
    int new_socket;
    admin admin;
    int addresslen                  = sizeof(proxy->admin_addr);
    char * g_message                = "Welcome to Proxy POP3 1.0\r\n";

    if ((new_socket = accept((*proxy).admin_socket, (struct sockaddr *)&(*proxy).admin_addr, (socklen_t *)&addresslen))<0) {
        perror("Accept error ocurred!\n");
        exit(EXIT_FAILURE);
    }

    printf("\n[ADMIN] New connection:\n\t - Socket FD: %d \n\t - IP: %s \n\t - Port: %d \n", new_socket, inet_ntoa((*proxy).mua_addr.sin_addr), ntohs((*proxy).mua_addr.sin_port));

    /* Send new connection greeting message */
    if (send(new_socket, g_message, strlen(g_message), 0) != strlen(g_message)) {
        perror("An error ocurred trying to send greeting messsage!");
    } else {
        printf("Greeting message was sent successfully to socket %d\n", new_socket);
    }

    admin.admin_fd = new_socket;
    admin.current_status = WAITING_USER;

    request_username(new_socket);

    return admin;
}


void request_username(file_descriptor socket){
    char * message = "Please, to connect insert USER:\r\n";
    /* Send new connection greeting message */
    if (send(socket, message, strlen(message), 0) != strlen(message)) {
        perror("An error ocurred trying to send messsage!");
    } else {
        printf("Message was sent successfully to socket %d\n", socket);
    }
}