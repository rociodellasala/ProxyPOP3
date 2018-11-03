#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "include/filter_list.h"

/*
*	Si tenemos prohibido image/png desps ponemos image/* lo reemplaza y funca.
*	Si tenemos prohibido image/* y queremos ALLOW image/png no se puede.
*	
*/

/*
int main(void){

	struct filter_list* list = filter_list_init();
	forbid_new("image", "png", list);
	forbid_new("text", "plain", list);
	//forbid_new("image", "*", list);
	allow_type("image", "*", list);

	printf("%s\n", get_forbidden_types(list));
	return 0;
}*/


struct filter_list* filter_list_init(){
	struct filter_list* list = malloc(sizeof(struct filter_list));
	if(list == NULL){
		return NULL;
	}
	list->first = NULL;
	return list;
}


int forbid_new(char* type, char* subtype, struct filter_list* list){
	struct type_node* curr_type;
	bool type_found = false;
	bool subtype_found = false;
	//while(curr_type != NULL){
		curr_type = search_for_type(type, list, &type_found);
		if(type_found){

			if(curr_type->wildcard){
				return 0; //no hace falta agregarlo
			}
			if(strcasecmp(subtype, "*") == 0){
				make_subtype_wildcard(curr_type);
				return 0;
			}

			struct subtype_node* curr_subtype;
			curr_subtype = search_for_subtype(type, curr_type, &subtype_found);
			if(subtype_found){
				return 0; //ya estaba ese subtipo
			}
			curr_subtype = malloc(sizeof(struct subtype_node));

			if(curr_subtype == NULL){
				if(curr_type == NULL){
					delete_media_type(list, type);
				}
				return -1; //error
			}

			curr_subtype->name = subtype;
			curr_subtype->next = NULL;
			if(curr_type->first == NULL){
				curr_type->first = curr_subtype;
			}else{
				//curr_subtype
				struct subtype_node* aux = curr_type->first;
				while(aux->next != NULL){
					aux = aux->next;
				}
				aux->next = curr_subtype;
				curr_subtype->next = NULL;
			}
			return 0;
		}else{
			//no encontro el tipo
			
			curr_type = malloc(sizeof(struct type_node));
			if(curr_type == NULL){
				return -1;
			}
			curr_type->name = type;
			curr_type->next = NULL;
			curr_type->first = NULL;
			curr_type->wildcard = false;

			if(list->first == NULL){
				list->first = curr_type;
			}else{
				struct type_node* aux = list->first;
				while(aux->next != NULL){
					aux = aux->next;
				}
				aux->next = curr_type;
			}

			if(strcasecmp(subtype, "*") == 0){
				make_subtype_wildcard(curr_type);
				return 0;
			}

			struct subtype_node* curr_subtype = malloc(sizeof(struct subtype_node));

			if(curr_subtype == NULL){
				if(curr_type == NULL){
					delete_media_type(list, type);
				}
				return -1; //error
			}

			curr_subtype->name = subtype;
			curr_subtype->next = NULL;
			curr_type->first = curr_subtype;
			return 0;
		}
	//}
}

struct subtype_node* search_for_subtype(char* subtype, struct type_node* type, bool* subtype_found){
	struct subtype_node* curr = type->first;
	while(curr != NULL){
		if(strcasecmp(curr->name, subtype) == 0){
			*subtype_found = true;
			return curr;
		}
		curr = curr->next;
	}
	subtype_found = false;
	return curr;
}

struct type_node* search_for_type(char* type, struct filter_list* list, bool* type_found){
	struct type_node* curr = list->first;
	while(curr != NULL){
		if(strcasecmp(curr->name, type) == 0){
			*type_found = true;
			return curr;
		}
		curr = curr->next;
	}
	type_found = false;
	return curr;
}

void make_subtype_wildcard(struct type_node* node){
	struct subtype_node* curr = node->first;
	struct subtype_node* aux = NULL;

	while(curr != NULL){
		aux = curr->next;
		//free(curr->name);
		free(curr);
		curr = aux;
	}
	node->first = NULL;
	node->wildcard = true;
	return;
}

int delete_media_type(struct filter_list* list, char* type){
	struct type_node* curr = list->first;
	struct type_node* aux = NULL;
	struct type_node* prev = NULL;

	while(curr != NULL){
		aux = curr->next;
		if(strcasecmp(curr->name, type) == 0){
			if(prev == NULL){
				list->first = aux;
			}else{
				prev->next = aux;
			}
			//free(curr->name);
			free(curr);
		}
		prev = curr;
		curr = aux;

	}
	return 0;
}

int allow_type(char* type, char* subtype, struct filter_list* list){

	bool type_found = false;
	bool subtype_found = false;

	struct type_node* curr_type = list->first;

	curr_type = search_for_type(type, list, &type_found);
	if(type_found == false){
		return -1;
	}
	if(strcmp("*", subtype) == 0){
		//borrar completamente el tipo
		// ya que ahora se permite todo
		completely_allow_type(list, curr_type);
		return 1;
	}
	if(curr_type->wildcard){
		return -1;//no se puede permitir un subtipo si
				// antes se prohibieron todos los subtipos
	}
	struct subtype_node* n = curr_type->first;
	struct subtype_node* aux = NULL;
	struct subtype_node* prev = NULL;
	while(n != NULL){
		aux = n->next;
		if(strcasecmp(n->name, subtype) == 0){
			if(prev == NULL){
				curr_type->first = aux;
			}else{
				prev->next = aux;
			}
			//free(n->name);
			free(n);
			return 1;
		}
		prev = n;
		n = aux;
	}
	return -1;
}

void completely_allow_type(struct filter_list* list, struct type_node* type){
	//primero libero los hijos
	struct subtype_node* subtype = type->first;
	struct subtype_node* aux = NULL;
	while(subtype != NULL){
		aux = subtype->next;
		//free(subtype->name);
		free(subtype);
		subtype = aux;
	}	

	if(strcasecmp(list->first->name, type->name) == 0){
		//free(type->name);
		if(type->next == NULL){
			list->first = NULL;
		}else{
			list->first = type->next;
		}
		free(type);
		
		return;
	}

	struct type_node* prev = list->first;
	while(!strcasecmp(prev->next->name, type->name) == 0){
		prev = prev->next;
	}

	prev->next = type->next;

	//free(type->name);
	free(type);
	return;

}

char* get_forbidden_types(struct filter_list* list){
	char* str = malloc(BUFFER*sizeof(char));
	size_t size = BUFFER;
	size_t index = 0;
	struct type_node* node = list->first;

	while(node != NULL){
		struct subtype_node* n = node->first;
		size_t type_length = strlen(node->name);

		if(node->wildcard){


			size_t subtype_length =	strlen("*");

			if(size <= index + type_length + subtype_length + 2){
				size_t growth = ((size - index)/ BUFFER + 2)*BUFFER;
				void *aux = realloc(str, size + growth);
				if(aux == NULL){
					return NULL;
				}
				str = aux;
				size += growth;
			}
			strcpy(str + index, node->name);
			index = index + type_length;
			str[index++] = '/';
			strcpy(str + index, "*");			
			index = index + subtype_length;
			str[index++] = ',';

		}

		while(n != NULL){

			size_t subtype_length = strlen(n->name);

			if(size <= index + type_length + subtype_length + 2){
				size_t growth = ((size - index)/ BUFFER + 2)*BUFFER;
				void *aux = realloc(str, size + growth);
				if(aux == NULL){
					return NULL;
				}
				str = aux;
				size += growth;
			}
			strcpy(str + index, node->name);
			index = index + type_length;
			str[index++] = '/';
			strcpy(str + index, n->name);			
			index = index + subtype_length;
			str[index++] = ',';
			n = n->next;
		}
		node = node->next;
	}

	if(index != 0){
		str[index-1] = '\0';
	}else{
		str[index] = '\0';
	}

	return str;
}

int check_mime_format(char* str, char** type, char** subtype){

	bool slash = false;
	*type = str;

	for(int i = 0; str[i] != 0; i++){
		if(str[i] == '/' && !slash){
			str[i] = '\0';
			slash = true;
			if(str[i+1] != 0){
				*subtype = str + i + 1;
			}else{
				return -1;
			}
		}else if(str[i] == '/' && slash){
			return -1;
		}
	}
	if(slash){
		return 1;
	}else{
		return -1;
	}

}

bool find_mime(struct filter_list* list, char* type, char* subtype){
	bool type_found = false;
	bool subtype_found = false;

	struct type_node* aux = search_for_type(type, list, &type_found);

	if(subtype_found && type_found){
		return true;
	}

	return false;

}

