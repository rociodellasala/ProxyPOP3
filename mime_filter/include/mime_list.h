#ifndef PROXYPOP3_MIMELIST_H
#define PROXYPOP3_MIMELIST_H
#include "parser_creator.h"

/*
*	Lista donde tenemos nodos que almacenan los MIME types a censurar.
*	Tenemos nodos tipo quienes a su vez tienen una lista de nodos subtipos.
*	Cada nodo almacena un puntero al parser y su definición. 
*
*/

/* Nodo tipo que tiene un puntero al listado de subtipos censurados. */
struct type_node {
	struct parser * 			parser;
	struct parser_definition * 	def;
	struct type_node * 			next;
	struct subtype_node * 		subtypes;
	const char * 				name;
	const struct parser_event * event;
	bool 						wildcard;
};

/* Nodo subtipo */
struct subtype_node {
	struct parser * 			parser;
	struct parser_definition * 	def;
	struct subtype_node * 		next;
	const char * 				name;
	const struct parser_event * event;
	bool 						wildcard;
};

/* Listado de nodos tipo, los cuales a su vez contienen un listado de subtipos. */
struct node_list {
	struct type_node * 			first;
};

/* Inicialización del listado. */
struct node_list * create_list(void);

/* Agrega nuevo MIME type al listado. */
int add_new(char *, char *,struct node_list *);

/* Busca si existe un nodo tipo en el listado correspondiente al type que le pasan por parametro. */
struct type_node * search_for_type(struct node_list *, char *, bool *);

/* Busca si existe un nodo subtipo en el listado de subtipos correspondiente al subtype que le pasan por parametro. */
struct subtype_node * search_for_subtype(struct subtype_node *, char *, bool *);

/* Crea un nuevo nodo tipo con el nombre que le pasan y lo retorna. */
struct type_node * create_new_type(char *);

/* Crea un nuevo nodo subtipo con el nombre que le pasan y lo retorna. */
struct subtype_node * create_new_subtype(char *);

/* Imprime los MIME types a censurar. */
//void print_list(struct node_list *);

/* Destruye la lista realizando los free correspondientes. */
void destroy_list(struct node_list *);

/* Resetea todos los parsers de la lista. */
void clean_list(struct node_list *);

/* Retorna un nodo subtipo que sea wildcard */
struct subtype_node * create_new_wildcard_subtype();

/* Remueve todos los subtipos que tenia almacenado el nodo tipo que recibe y hace que el subtipo pase a ser wildcard */
void make_subtype_wildcard(struct type_node *, char *, bool);

#endif