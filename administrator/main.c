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

