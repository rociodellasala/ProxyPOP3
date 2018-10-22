#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "mimeList.h"

#define WILDCARD "*"


struct List* create_list(void){

	struct List* list = malloc(sizeof(*list));
	if(list != NULL){
        memset(list, 0, sizeof(*list)); // copia el caracter 0 a los primeros sizeof(*list) caracteres de list
	}
	return list;
}

int add_new(char* type, char* subtype,struct List* list){

	if(list == NULL){
		return -1; //error
	}

	if(list->first != NULL){

		bool typeExists = false;
		bool subtypeExists = false;

		struct type_node*  tpnode = search_for_type(list,type,&typeExists);

		//FALTA VER QUE PASA SI subtype es *

		if(typeExists){

				struct subtype_node* stpnode = search_for_subtype(tpnode->subtypes, subtype, &subtypeExists);

				if(subtypeExists){
					return -1;
				}else{
					//agrego nuevo subtipo al nodo tipo
					stpnode->next = create_new_subtype(subtype);
					return 0;
				}
		}else{
				//agrego un nuevo tipo
				tpnode->next = create_new_type(type);
				tpnode->next->subtypes = create_new_subtype(subtype);
				return 0;
		}
				

	}else{
		//tengo que inicializar 

			list->first = create_new_type(type);
			list->first->subtypes = create_new_subtype(subtype);
			//FALTA VER QUE PASA SI subtype es *				
			return 0;
	}
}


struct type_node* search_for_type(struct List* list, char* type, bool* typeExists){
	struct type_node* current = list->first;
	if(strcmp(current->name,type) == 0){
		*typeExists = true;
		return current;
	}
	while(current->next != NULL){
		current = current->next;
		if(strcmp(current->name,type) == 0){
			*typeExists = true;
			return current;
		}
	}
	//sino lo encontre retorno el ultimo
	return current;
}

struct subtype_node* search_for_subtype(struct subtype_node* current, char* subtype, bool* subtypeExists){
	//FALTA VER QUE PASA SI subtype es *
	if(strcmp(current->name,subtype) == 0){
		*subtypeExists = true;
		return current;
	}
	while(current->next != NULL){
		current = current->next;
		if(strcmp(current->name,subtype) == 0){
			*subtypeExists = true;
			return current;
		}
	}
	return current;
}

struct type_node* create_new_type(char* name){

	struct type_node* node = malloc(sizeof(*node));

	if(node != NULL){
		memset(node,0,sizeof(*node)); 
		

		//node->parser = parser_init(parser_no_classes(), def);
		//node->def	= def;
		node->next = NULL;
		node->subtypes = NULL;
		node->name = name;
		//node->event = NULL;
		//node->wildcard = false;		
	}
	return node;
}

struct subtype_node* create_new_subtype(char* name){

	struct subtype_node* node = malloc(sizeof(*node));

	if(node != NULL){
		memset(node,0,sizeof(*node)); 
		

		//node->parser = parser_init(parser_no_classes(), def);
		//node->def	= def;
		node->next = NULL;
		node->name = name;
		//node->event = NULL;
		//node->wildcard = false;		
	}
	return node;
}

//FALTA remover subtypes de un type_node, remover un nodo, destruir nodo
// destrui y resetear parser
// el tema de las * en todos lados

//funcion de imprimir lista
void print_list(struct List* list){
	printf("Here's your list\n");

	struct type_node* current = list->first;
	struct subtype_node* currentSub; 

	while(current != NULL){
		printf("-----------------------------------------------------------\n");
		printf("Type: %s\n", current->name);
		currentSub = current->subtypes;
		while(currentSub != NULL){
			printf("-Subtype: %s \n", currentSub->name);
			currentSub = currentSub->next;
		}
		printf("\n");
		current = current->next;
	}

}




void removeNode(struct List* list, char* type, char*subtype){
	struct type_node* nodeType = list->first;
	struct type_node* tmpType;
	struct subtype_node* tmpSubType;
	struct subtype_node* nodeSubType;
	if(nodeType == NULL){
		return;
	}
	if(strcmp(nodeType->name,type) == 0){
			nodeSubType = nodeType->subtypes;
			if(strcmp(nodeSubType->name,subtype) == 0){
				nodeType->subtypes = nodeSubType->next;
				free(nodeSubType);
			}
			while(nodeSubType->next != NULL){
				tmpSubType = nodeSubType;
				nodeSubType = nodeSubType->next;
				if(strcmp(nodeSubType->name,subtype)==0){
					tmpSubType->next = nodeSubType->next;
					free(nodeSubType);
				}
			}
			if(nodeType->subtypes == NULL){
				tree->first = nodeType->next;
				free(nodeType);
				return;
			}
		while(nodeType->next!= NULL){
			tmpType = nodeType;
			nodeType = nodeType->next;
			if(strcmp(nodeType->name,type) == 0){
				nodeSubType = nodeType->subtypes;
				if(strcmp(nodeSubType->name,subtype) == 0){
					nodeType->subtypes = nodeSubType->next;
					free(nodeSubType);
				}
				while(nodeSubType->next != NULL){
					tmpSubType = nodeSubType;
					nodeSubType = nodeSubType->next;
					if(strcmp(nodeSubType->name,subtype)==0){
						tmpSubType->next = nodeSubType->next;
						free(nodeSubType);
					}
				}
				if(nodeType->subtypes == NULL){
					tmpType->next = nodeType->next;
					free(nodeType);
					return;
				}
			}
		}
	}
}

void destroy_list(struct List *list){
    struct type_node* node = list->first;
    struct subtype_node* subtypes;
    struct type_node* currType;
    struct subtype_node* currSubtype;

    while(node != NULL){
        subtypes = node->subtypes;
        while(subtypes != NULL){
            currSubtype = subtypes;
            if(subtypes->parser != NULL){
                parser_destroy(subtypes->parser);
                parser_utils_strcmpi_destroy(subtypes->def);
				free(subtypes->def);
            }
			if (!subtypes->wildcard)
				free((void*)subtypes->name);
            subtypes = subtypes->next;
            free(currSubtype);
        }
        currType = node;
        parser_destroy(node->parser);
        parser_utils_strcmpi_destroy(node->def);
		free(node->def);
		if (!node->wildcard)
			free((void*)node->name);
        node = node->next;
        free(currType);
    }
    free(list);
}

void clean_list(struct List* list){
    struct type_node* node = list->first;
    struct subtype_node* subtypes;
    while(node != NULL){
        subtypes = node->subtypes;
        while(subtypes != NULL){
			if (!subtypes->wildcard) {
				parser_reset(subtypes->parser);
			}
            subtypes = subtypes->next;
        }
        parser_reset(node->parser);
        node = node->next;
    }
}

struct subtype_node* newNodeWildcard(){
	struct subtype_node* node = malloc(sizeof(*node));
	if(node != NULL){
		memset(node,0,sizeof(*node));
		node->parser = NULL;
		node->def = NULL;
		node->next = NULL;
		node->name = WILDCARD;
		node->event = NULL;
		node->wildcard = true;
	}
	return node;
}

static void destroy_node(struct type_node *node) {
	if (node->name != NULL && node->wildcard) {
		free((void *) node->name);
	}
	if (node->def != NULL) {
		parser_utils_strcmpi_destroy(node->def);
		free(node->def);
	}
	if (node->parser != NULL) {
		parser_destroy(node->parser);
	}
}

static void destroy_node(struct subtype_node *node) {
	if (node->name != NULL && node->wildcard) {
		free((void *) node->name);
	}
	if (node->def != NULL) {
		parser_utils_strcmpi_destroy(node->def);
		free(node->def);
	}
	if (node->parser != NULL) {
		parser_destroy(node->parser);
	}
}