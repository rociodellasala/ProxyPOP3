#include <arpa/inet.h>
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
    admin.current_status = WAITING_EHLO;

    return admin;
}

void administrator_read(admin * admin){
    ssize_t valread;
    char buffer[BUFFER_SIZE];
    if ((valread = read((*admin).admin_fd, buffer, BUFFER_SIZE)) == 0) {
        /* An error ocurred while reading. Possibly the client has closed the connection. */
        // remove_administator_connection();
    } else {
        /* A client data has been read correctly. Set the string terminating
         * NULL byte on the end of the data read */
        buffer[valread] = '\0';
        printf("Mensaje del FD %d es: %s\n", (*admin).admin_fd, buffer);
        (*admin).current_status = parse_admin_command(*admin, buffer);
    }
}


void administrator_write(char * cmd, file_descriptor socket){
    if (send(socket, cmd, strlen(cmd), 0) != strlen(cmd)) {
        perror("An error ocurred trying to send greeting messsage!");
    } else {
        printf("Response message '%s' was sent successfully to socket %d\n",cmd, socket);
    }
}

void remove_administrator_connection(){

}

status parse_ehlo(char * cmd, admin admin){
    printf("cmd: %s\n", cmd);
    if(strcmp(cmd,"EHLO") == 0){
        administrator_write("PROXY: HELLO!\r\n", admin.admin_fd);
       
        return WAITING_USER;
    } else {
        administrator_write("PROXY: TRY AGAIN\r\n", admin.admin_fd);
        return WAITING_EHLO;
    }
}

status parse_user(char * cmd, admin admin){
    printf("cmd: %s\n", cmd);
    if(strcmp(cmd,"ADMINISTRATOR") == 0){
        administrator_write("PROXY: OK\r\n", admin.admin_fd);
        return WAITING_PASS;
    } else {
        administrator_write("PROXY: TRY AGAIN\r\n", admin.admin_fd);
        return WAITING_USER;
    }
}


status parse_admin_command(admin admin, char ** cmd){
    status new_status;
    status current_status = admin.current_status;
    printf("aca status: %d\n",admin.current_status );
    switch (current_status) {
        case WAITING_EHLO:
            new_status = parse_ehlo(cmd, admin);
            break;
        case WAITING_USER:
            new_status =  parse_user(cmd, admin);
            break;
        case WAITING_PASS:
            //parse_pass();
            break;
        default:
            break;
    }
}

