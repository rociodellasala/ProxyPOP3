#ifndef MIMELIST_H_
#define MIMELIST_H_
#include "parser_creator.h"

struct type_node{
	struct parser* parser;
	struct parser_definition *def;
	struct type_node *next;
	struct subtype_node *subtypes;
	const char* name;
	const struct parser_event* event;
	bool wildcard;
};

struct subtype_node{
	struct parser* parser;
	struct parser_definition *def;
	struct subtype_node *next;
	const char* name;
	const struct parser_event* event;
	bool wildcard;
};

struct List{
	struct type_node* first;
};

struct mime{
	char* type;
	char* subtype;
};

struct List* create_list(void);

int add_new(char* type, char* subtype,struct List* list);

struct type_node* search_for_type(struct List* list, char* type, bool* typeExists);

struct subtype_node* search_for_subtype(struct subtype_node* current, char* subtype, bool* subtypeExists);

struct type_node* create_new_type(char* name);

struct subtype_node* create_new_subtype(char* name);

void print_list(struct List* list);

void removeNode(struct List* list, char* type, char*subtype);

void destroy_list(struct List *list);

void clean_list(struct List* list);

struct subtype_node* newNodeWildcard();

//static void destroy_node(struct type_node *node);

static void destroy_node(struct subtype_node *node);

void addWildcard(struct type_node* node, char* type, bool typeExists);

#endif