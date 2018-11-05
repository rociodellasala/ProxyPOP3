#ifndef FILTER_LIST_H
#define FILTER_LIST_H

#include <unistd.h>
#include <stdbool.h>

#define BUFFER 100

struct type_node{
	char* name;
	struct type_node* next;
	struct subtype_node* first;
	bool wildcard;
};

struct subtype_node{
	struct subtype_node* next;
	char* name;
};

struct filter_list{
	struct type_node* first;
};

struct filter_list* filter_list_init();
int forbid_new(char* type, char* subtype, struct filter_list* list);
struct subtype_node* search_for_subtype(char* subtype, struct type_node* type, bool* subtype_found);
struct type_node* search_for_type(char* type, struct filter_list* list, bool* type_found);
void make_subtype_wildcard(struct type_node* node);
int delete_media_type(struct filter_list* list, char* type);
int allow_type(char* type, char* subtype, struct filter_list* list);
void completely_allow_type(struct filter_list* list, struct type_node* type);
char* get_forbidden_types(struct filter_list* list);
int check_mime_format(char* str, char** type, char** subtype);
bool find_mime(struct filter_list* list, char* type, char* subtype);

#endif 