#ifndef PROXYPOP3_ADMIN_ACTIONS_H
#define PROXYPOP3_ADMIN_ACTIONS_H

int check_password(const char *);
void return_metric(struct admin *, const char *);
void switch_transformation_program(struct admin *);
//void forbid_mime(request *, int *, char *);
//void allow_mime(request *, int *, char *);
void quit_admin(struct admin *);

#endif //PROXYPOP3_ADMIN_ACTIONS_H
