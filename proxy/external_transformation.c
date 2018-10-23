#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "include/external_transformation.h"

char * concat_string(char * variable, char * value) {
    char * ret =  malloc((strlen(variable) + strlen(value)+1)* sizeof(char));
    strcat(ret, variable);
    strcat(ret, value);

    return ret;
}

char * transform(char * mail, char * replacement_msg, char * filtered_media_types, char * version,
                  char * username, char * server) {

    char *filter_medias         = concat_string("FILTER_MEDIAS=", filtered_media_types);
    char *filter_msg            = concat_string("FILTER_MSG=", replacement_msg);
    char *pop3_filter_version   = concat_string("POP3_FILTER_VERSION=", version);
    char *pop3_username         = concat_string("POP3_USERNAME=", username);
    char *pop3_server           = concat_string("POP3_SERVER=", server);

    pid_t pid;
    char *const parameters[]    = {"/bin/echo", mail, NULL};
    char *const environment[6]  = {filter_medias, filter_msg,
                                  pop3_filter_version,
                                  pop3_username, pop3_server, NULL};

    if ((pid = fork()) == -1) {
        perror("fork error");
    } else if (pid == 0) {
        execve("/bin/echo", parameters, environment);
        perror("execve");
    }
}
