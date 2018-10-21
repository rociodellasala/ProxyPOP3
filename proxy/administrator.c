#include <arpa/inet.h>
#include <errno.h>
#include "include/administrator.h"
#include "include/main.h"
#include "include/request.h"
#include "include/deserializer.h"
#include "include/response.h"
#include "include/serializer.h"

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
    admin.current_status = WAITING_EHLO;

    return admin;
}

void administrator_read(admin * admin, options opt){
    ssize_t valread;
    char buffer[BUFFER_SIZE];
    if ((valread = read((*admin).admin_fd, buffer, BUFFER_SIZE)) == 0) {
        /* An error ocurred while reading. Possibly the client has closed the connection. */
        // remove_administator_connection();
    } else {
        /* A client data has been read correctly. Set the string terminating
         * NULL byte on the end of the data read */
        buffer[valread] = '\0';
        printf("Mensaje del FD %d \n", (*admin).admin_fd);
        admin_read_handler(admin, buffer, &opt);
    }
}

void admin_read_handler(admin * admin, char * buffer, options * opt){
    int status;
    request * request = malloc(sizeof(request));
    
    deserialize_request(buffer, request);
    
    switch(request->cmd){
        case AUTH:
            printf("PIDIENDO EL COMANDO AUTH\n");
            check_password(request, &status);
            send_response_without_data(admin->admin_fd, status);
            break;
        case SET_TRANSF:
            printf("PIDIENDO EL COMANDO SET_TRANSF\n");
            send_response_without_data(admin->admin_fd, status);
            break;
        case GET_TRANSF:
            printf("PIDIENDO EL COMANDO GET_TRANSF\n");
            send_response_without_data(admin->admin_fd, status);
            break;
        case SWITCH_TRANSF:
            printf("PIDIENDO EL COMANDO SWITCH_TRANSF\n");
            send_response_without_data(admin->admin_fd, status);
            break;
        case GET_METRIC:
            printf("PIDIENDO EL COMANDO GET_METRIC\n");
            check_password(request, &status);
            send_response_without_data(admin->admin_fd, status);
            break;
        case GET_MIME:
            printf("PIDIENDO EL COMANDO GET_MIME\n");
            status = 1;
            send_response_with_data(opt->filtered_media_types, admin->admin_fd, status);
            break;
        case ALLOW_MIME:
            printf("PIDIENDO EL COMANDO ALLOW_MIME\n");
            //allow_mime(request, &status, opt->filtered_media_types);
            send_response_without_data(admin->admin_fd, status);
            break;
        case FORBID_MIME:
            printf("PIDIENDO EL COMANDO FORBID_MIME\n");
            //forbid_mime(request, &status, opt->filtered_media_types);
            send_response_without_data(admin->admin_fd, status);
            break;
        case QUIT:
            printf("PIDIENDO EL COMANDO QUIT\n");
            //liberar_todos_los_recursos_del_admin();
            send_response_without_data(admin->admin_fd, status);
            break;
    }

}


