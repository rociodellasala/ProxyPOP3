#ifndef PROXYPOP3_ADMIN_ACTIONS_H
#define PROXYPOP3_ADMIN_ACTIONS_H

/**
 * Funciones que resuelven los pedidos del admin
 * y si es necesario retornan un valor
 */
int check_password(const char *);
void return_metric(struct admin *, const char *);
void switch_transformation_program(struct admin *);
void forbid_mime(struct request_admin * request, enum parse_req_status * status);
void allow_mime(struct request_admin * request, enum parse_req_status * status);
void quit_admin(struct admin *);

#endif //PROXYPOP3_ADMIN_ACTIONS_H
