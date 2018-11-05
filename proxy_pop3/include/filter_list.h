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

struct filter_list * filter_list_init();
int forbid_new(char * type, char *, struct filter_list * );
struct subtype_node* search_for_subtype(char *, struct type_node *, bool *);
struct type_node* search_for_type(char *, struct filter_list *, bool *);
void make_subtype_wildcard(struct type_node *);
int delete_media_type(struct filter_list *, char *);
int allow_type(char * type, char *, struct filter_list *);
void completely_allow_type(struct filter_list * , struct type_node *);
char* get_forbidden_types(struct filter_list * );
int check_mime_format(char *, char **, char **);
bool find_mime(struct filter_list*, char*, char *);

#endif 