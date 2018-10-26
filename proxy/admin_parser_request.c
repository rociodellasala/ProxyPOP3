
#include "include/input_parser.h"
#include "include/deserializer.h"

typedef int file_descriptor;

void parse_admin_request(struct admin * admin) {
    int status;
    int rd_sz;
    file_descriptor admin_fd = admin->fd;
    request_admin * request = malloc(sizeof(request));
    unsigned char buffer[40];

    rd_sz = sctp_recvmsg(key->fd, buffer, 40, NULL,0,0,0);

    if (rd_sz <= 0) {
        printf("%s\n", strerror(errno));
    }

    deserialize_request(buffer, request);
}

static struct parse_action hello_action = {
        .status      = ST_EHLO,
        .action      = parse_hello,
        .args        = 0,
};

static struct parse_action user_action = {
        .status      = ST_USER,
        .action      = parse_user,
        .args        = 2,
};

static struct parse_action pass_action = {
        .status      = ST_PASS,
        .action      = parse_pass,
        .args        = 2,
};

static struct parse_action config_action = {
        .status      = ST_CONFIG,
        .action      = parse_config,
        .args        = 0,
};

static struct parse_action * action_list[] = {
        &hello_action,
        &user_action,
        &pass_action,
        &config_action,
};
