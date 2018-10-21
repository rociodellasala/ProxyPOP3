
#include "include/administrator.h"
#include "include/request.h"
#include "include/send_request.h"
#include "include/response.h"
#include "include/receive_response.h"
#include "include/utils.h"

void communicate_with_proxy(file_descriptor socket) {
    int i, j;
    unsigned char status = 0;
    char buffer_option[MAX_BUFFER];
    char param[MAX_BUFFER];
    enum cmd c;
    int correct_option;
    int running                  = 1;
    int flag_quit_option              = 0;

    show_menu();

    while (running) {
        printf("\nInsert a command to run on proxy: ");
        correct_option = 1;

        if (fgets(buffer_option, MAX_BUFFER, stdin) == NULL){
            close(socket);
            exit(-1);
        }

        if (buffer_option < 0) {
            printf("Error while reading from stdin\n");
            close(socket);
            exit(-1);
        }

        i = 0;
        c = buffer_option[i];

        switch (c){
            case A:
                fun(i, j, buffer_option, param, &correct_option, socket, AUTH);
                break;
            case SET_T:
                fun(i, j, buffer_option, param, &correct_option, socket, SET_TRANSF);
                break;
            case GET_T:
                i++;
                if(buffer_option[i] != NEWLINE){
                    correct_option = 0;
                    break;
                } else {
                    send_request_without_param(GET_TRANSF, socket);
                    break;
                }
            case SWITCH_T:
                fun(i, j, buffer_option, param, &correct_option, socket, SWITCH_TRANSF);
                break;
            case GET_ME:
                fun(i, j, buffer_option, param, &correct_option, socket, GET_METRIC);
                break;
            case GET_MI:
                i++;
                if(buffer_option[i] != NEWLINE){
                    correct_option = 0;
                    break;
                } else {
                    send_request_without_param(GET_MIME, socket);
                    break;
                }
            case ALLOW_MI:
                fun(i, j, buffer_option, param, &correct_option, socket, ALLOW_MIME);
                break;
            case FORBID_MI:
                fun(i, j, buffer_option, param, &correct_option, socket, FORBID_MIME);
                break;
            case Q:
                i++;
                if(buffer_option[i] != NEWLINE){
                    correct_option = 0;
                    break;
                } else {
                    flag_quit_option = 1;
                    send_request_without_param(QUIT, socket);
                    break;
                }
            default:
                correct_option = 0;
                break;
        }

        if(correct_option == 1){
            response response;
            ssize_t recv_bytes;
            recv_bytes = receive_response(socket,&response,&status);
            if (recv_bytes < 0) {
                printf("error receiving\n");
                return;
            }
            print_msg(status, response);
        }

        if(flag_quit_option == 1 && status == 1){
            running = 0;
        }
    }

    printf("See you later!\n");

}

void fun(int i, int j, unsigned char * buffer_option, unsigned char * param, int * correct_option, file_descriptor socket, enum cmd command) {
    i++;
    if(buffer_option[i] != SPACE){
        *correct_option = 0;
    } else {
        i++;
        j = 0;
        for (; i < MAX_READ && buffer_option[i] != '\n'; i++) {
            param[j++] = buffer_option[i];
        }
        send_request_one_param(command, param, socket);
    }
}

