#ifndef FILTER_LIST_H
#define FILTER_LIST_H

#include <unistd.h>
#include <stdbool.h>

#define BUFFER 100


struct type_node{
	char * 					name;
	struct type_node * 		next;
	struct subtype_node * 	first;
	bool 					wildcard;
};


struct subtype_node{
	struct subtype_node * 	next;
	char * 					name;
};

struct filter_list{
	struct type_node * 		first;
};

/* Inicializa el listado de nodos tipo. */
struct filter_list * filter_list_init();

/* Agrega un nuevo MIME type prohibido. */
int forbid_new(char * type, char *, struct filter_list * );

/* Busca un subtipo dentro de un nodo tipo. */
struct subtype_node* search_for_subtype(char *, struct type_node *, bool *);

/* Busca un nodo tipo en la lista. */
struct type_node* search_for_type(char *, struct filter_list *, bool *);

/* Hace que el subtipo de un nodo pase a ser wildcard. */
void make_subtype_wildcard(struct type_node *);

/* Borra un nodo tipo. */
int delete_media_type(struct filter_list *, char *);

/* Borra un MIME type de la lista. */
int allow_type(char * type, char *, struct filter_list *);

/* Borra un nodo tipo de la lista con todos sos subtipos. */
void completely_allow_type(struct filter_list * , struct type_node *);

/* Devuelve un string con los MIME type prohibidos. */
char* get_forbidden_types(struct filter_list * );

/* Detecta el formato correcto de un MIME type y los valores correspondientes para el tipo y subtipo en los parametros que recibe. */
int check_mime_format(char *, char **, char **);

/* BBusca un MIME type en el listado. */
bool find_mime(struct filter_list*, char*, char *);

#endif 