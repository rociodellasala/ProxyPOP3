#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "include/administrator.h"
#include "include/request.h"
#include "include/send_request.h"
#include "include/response.h"
#include "include/receive_response.h"
#include "include/utils.h"

void handle_receive_msg(unsigned char * status, const file_descriptor socket) {
    ssize_t recv_bytes;
    response response;
    recv_bytes = receive_response(status, &response, socket);

    if (recv_bytes < 0) {
        printf("An error occured while receiving message from proxy.\n");
        return;
    }

    print_msg(*status, response);

    if (response.length > 0) {
        free(response.data);
    }
}

void assemble_req(int i, const char * buffer_option, int * correct_option, const file_descriptor socket, const enum cmd command) {
    char param[MAX_BUFFER] = {0};
    int j = 0;

    if(buffer_option[i] != SPACE){
        *correct_option = 0;
    } else {
        i++;

        for (; i < MAX_READ && buffer_option[i] != '\n'; i++) {
            param[j++] = buffer_option[i];
        }

        send_request_one_param(param, command, socket);
    }
}

void communicate_with_proxy(const file_descriptor socket) {
    char buffer_option[MAX_BUFFER];
    int i;
    int correct_option;
    enum cmd c;
    unsigned char status                = 0;
    int running                         = 1;
    int flag_quit_option                = 0;

    show_menu();

    while (running) {
        printf("\nInsert a command to run on proxy: ");
        correct_option = 1;

        if (fgets(buffer_option, MAX_BUFFER, stdin) == NULL) {
            close(socket);
            exit(-1);
        }

        if (buffer_option < 0) {
            printf("An error occured while reading from STDIN.\n");
            close(socket);
            exit(EXIT_FAILURE);
        }

        i = 0;
        c = (enum cmd) buffer_option[i];

        switch (c) {
            case A:
                assemble_req(++i, buffer_option, &correct_option, socket, A);
                break;
            case SET_T:
                assemble_req(++i, buffer_option, &correct_option, socket, SET_T);
                break;
            case GET_T:
                i++;
                if (buffer_option[i] != NEWLINE) {
                    correct_option = 0;
                    break;
                } else {
                    send_request_without_param(GET_T, socket);
                    break;
                }
            case SWITCH_T:
                i++;
                if (buffer_option[i] != NEWLINE) {
                    correct_option = 0;
                    break;
                } else {
                    send_request_without_param(SWITCH_T, socket);
                    break;
                }
            case GET_ME:
                assemble_req(++i, buffer_option, &correct_option, socket, GET_ME);
                break;
            case GET_MI:
                i++;
                if (buffer_option[i] != NEWLINE) {
                    correct_option = 0;
                    break;
                } else {
                    send_request_without_param(GET_MI, socket);
                    break;
                }
            case ALLOW_MI:
                assemble_req(++i, buffer_option, &correct_option, socket, ALLOW_MI);
                break;
            case FORBID_MI:
                assemble_req(++i, buffer_option, &correct_option, socket, FORBID_MI);
                break;
            case Q: /* TODO : QUIT EN EL CASO QUE TODAVIA NO AUNTENTIQUE, COMANDOS POR STATUS EN SHOW */
                i++;
                if (buffer_option[i] != NEWLINE) {
                    correct_option = 0;
                    break;
                } else {
                    flag_quit_option = 1;
                    send_request_without_param(Q, socket);
                    break;
                }
            case HELP:
                show_menu();
                correct_option = 0;
                break;
            default:
                correct_option = 0;
                printf("The entered command does not exist.\n");
                break;
        }

        if (correct_option == 1) {
            handle_receive_msg(&status,socket);
        } else {
            printf("Incorrect sintax of command. Press HELP option (0) to display menu again.\n");
        }

        if(flag_quit_option == 1 && status == 1){
            running = 0;
        }
    }

    printf("See you later!\n");
}
