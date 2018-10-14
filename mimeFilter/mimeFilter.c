#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//variables de entorno del manual pop3filter.8
#define FILTER_MEDIAS 	"FILTER_MEDIAS"
#define FILTER_MSG 		"FILTER_MSG"

int main(int argc, char ** argv) {

	/*
	* getenv(const char *name) searches for the environment string 
	* pointed to by name and returns the associated value to the string. 
	*/

	//char* flm = getenv("FILTER_MEDIAS");

	char* flm = "image/jpeg";

	//struct  Tree* mime_tree = tree_init();

	if(flm == NULL){
		printf("Unable to get filter medias\n");
		return -1;
	}
	printf("flm is %s\n", flm);
	
	
	char* medias = malloc(strlen(flm) + 1);
	
	if(medias == NULL){
		printf("bye 1\n");
		return -1;
	}

	strcpy(medias, flm);

	const char *comma = ",";
	const char *slash = "/";
	char *context = "comma";
	char *context_b = "slash";



	char *current;
	char *mime;
	
	/*tomar primer media*/
	
	current = strtok_r(medias, comma, &context);

	while(current != NULL){
		printf("INSIDE WHILE\n");
		char *aux = malloc(strlen(current) + 1);
		if(aux == NULL){
			printf("bye aux\n");
			return -1;
		}
		strcpy(aux, current);
	
		/* getting type */
		
		mime = strtok_r(aux, slash, &context_b);
		if(mime == NULL){
			printf("bye mime\n");
			return -1;
		}

		char *type = malloc(strlen(mime) + 1);
		if(type == NULL){
			printf("bye type\n");
			return -1;
		}
		strcpy(type, mime);
		printf("Ok type es %s\n", type);
		/*getting subtype*/
		
		mime = strtok_r(NULL, slash, &context_b);
		if(mime == NULL){
			printf("bye mime2\n");
			return -1;
		}

		char *subtype = malloc(strlen(mime) + 1);

		if(subtype == NULL){
			printf("bye subtpe\n");
			return -1;
		}

		strcpy(subtype, mime);
		printf("ok subtype es %s\n", subtype);
		// trees

		free(aux);
		current = strtok_r(NULL, comma, &context);
	}
	printf("outside of while\n");
	// free(flm); no funca

	return 0;
	//return stripmime(argc, argv, tree);
	
}

