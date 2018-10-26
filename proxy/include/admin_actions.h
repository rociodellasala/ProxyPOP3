#ifndef PROXYPOP3_ADMIN_ACTIONS_H
#define PROXYPOP3_ADMIN_ACTIONS_H

#include "selector.h"

typedef int file_descriptor;

int check_password(const unsigned char *);
void return_metric(struct admin *, const char *);
//void forbid_mime(request *, int *, char *);
//void allow_mime(request *, int *, char *);
void quit(struct admin *);


#endif //PROXYPOP3_ADMIN_ACTIONS_H
