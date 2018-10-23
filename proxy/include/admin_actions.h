#ifndef PROXYPOP3_ADMIN_ACTIONS_H
#define PROXYPOP3_ADMIN_ACTIONS_H

typedef int file_descriptor;

int check_password(request *, int *);
void forbid_mime(request *, int *, char *);
void allow_mime(request *, int *, char *);

ssize_t send_response(file_descriptor, response *);
void send_response_without_data(file_descriptor, int);
void send_response_with_data(char *, file_descriptor, int);

#endif //PROXYPOP3_ADMIN_ACTIONS_H
