#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "mimeList.h"
#include "mimeFilter.h"

//variables de entorno del manual pop3filter.8
#define FILTER_MEDIAS 	"FILTER_MEDIAS"
#define FILTER_MSG 		"FILTER_MSG"
#define ANY (1 << 9) //cambiar esto y tmbn en la funcion

static const unsigned classes[0xFF] = {0x00};

int main(int argc, char ** argv) {

	/*
	* getenv(const char *name) searches for the environment string 
	* pointed to by name and returns the associated value to the string. 
	*/

	//char* flm = getenv("FILTER_MEDIAS");

	char* flm = "image/jpeg,image/gif,image/png,text/plain,text/html";

	struct List *list = create_list();

    if (list == NULL)
        return -1;

	if(flm == NULL){
		printf("Unable to get filter medias\n");
		free(list);
		return -1;
	}
	printf("flm is %s\n", flm);
	
	
	char* medias = malloc(strlen(flm) + 1);
	
	if(medias == NULL){
		printf("bye 1\n");
		free(list);
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

	//en este WHILE faltan hacer free! con variable error porque sino hay que destruir  el mimeparser list
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

		// free ?
		int addition = add_new(type, subtype, list);
		if(addition != -1){
			printf("Node correctly added!\n");
		}

		free(aux);
		current = strtok_r(NULL, comma, &context);
	}
	printf("outside of while\n");
	// free(flm); no funca

	print_list(list);
	
	return 0;

	
	char *message = getenv(FILTER_MSG);

    if (message == NULL) {
        message = "Parte reemplazada.";
    }
	

	const unsigned int *no_class = parser_no_classes(); // Cambiar esto

    struct parser_definition ctypeParser = parser_init(no_class, create_parser_def_for_string("content-type")); 

    struct parser_definiton boundaryParser = parser_init(no_class, create_parser_def_for_string("boundary")); 

    struct parser_definition messageParser = parser_init(init_char_class(), mime_message_parser()); 

    struct parser_definiton multiParser =parser_init(no_class, pop3_multi_parser());  

    struct parser mimeTypeParser = parser_init(init_char_class(), mime_type_parser());  //falta este -- cambiarlo!

    //inicializo estructura
	//falta hacer

    struct currentSituation currSitutation = {
            .multi                  = multiParser,
            .msg                    = messageParser,
            .ctype_header           = ctypeParser,
            .mime_type              = mimeTypeParser,
            .boundary               = boundaryParser,
            .mime_list              = list,
            .boundary_frontier      = stack_new(),
            .filtered_msg_detected  = NULL,
            .boundary_detected      = NULL,
            .frontier_end_detected  = NULL,
            .frontier_detected      = NULL,
            .filter_msg             = filter_msg,
            .replace                = false,
            .replaced               = false,
            .buffer                 = {0},
            .i                      = 0,
    };

	uint8_t data[4096];
    ssize_t n;
    int fd = STDIN_FILENO;
    
    do {
        n = read(fd, data, sizeof(data));
        for (ssize_t i = 0; i < n; i++) {
            //pop3_multi(&ctx, data[i]); to do
        }
    } while (n > 0);


    //FALTAN destroys

    return 0;

}



const unsigned *parser_no_classes(void) {  //cambiar esto
    return classes;
}

struct parser_definition create_parser_def_for_string(const char *string) {

    const size_t length = strlen(string);

    struct transition **states   = calloc(n + 2, sizeof(*states)); //cambiar estos
    size_t *nstates = calloc(n + 2, sizeof(*nstates));
    struct transition *transitions= calloc(3 *(n + 2), sizeof(*transitions));

    if(states == NULL || nstates == NULL || transitions == NULL) {

        free(states);
        free(nstates);
        free(transitions);

        struct parser_definition ret = {
        	.start_state   = 0,
            .states_count  = 0,
            .states        = NULL,
            .states_n      = NULL,                  
        };
        return ret;
    }

    // estados fijos
    const size_t acceptState  = length;
    const size_t errorTrapState = length + 1;

    for(size_t i = 0; i < length; i++) {

        const size_t go;

        if((i + 1) == length){
        	go = acceptState;
        }else{
        	go = i + 1;
        }

        transitions[i * 3 + 0].received = tolower(string[i]);
        transitions[i * 3 + 0].go = go;
        transitions[i * 3 + 0].act1 = eq;
        transitions[i * 3 + 1].received = toupper(string[i]);
        transitions[i * 3 + 1].go = go;
        transitions[i * 3 + 1].act1 = eq;
        transitions[i * 3 + 2].received = ANY;
        transitions[i * 3 + 2].go = errorTrapState;
        transitions[i * 3 + 2].act1 = neq;
        states     [i] = transitions + (i * 3 + 0);
        nstates    [i] = 3;
    }

    // Agrego el estado EQUAL
    transitions[(length + 0) * 3].received   = ANY;
    transitions[(length + 0) * 3].go   = errorTrapState;
    transitions[(length + 0) * 3].act1   = neq;
    states     [(length + 0)] = transitions + ((length + 0) * 3 + 0);
    nstates    [(length + 0)] = 1;

    // Agrego el estado de error
    transitions[(length + 1) * 3].received  = ANY;
    transitions[(length + 1) * 3].go   = errorTrapState;
    transitions[(length + 1) * 3].act1   = neq;
    states     [(length + 1)] = transitions + ((n + 1) * 3 + 0);
    nstates    [(length + 1)] = 1;


    struct parser_definition ret = {

        .start_state   = 0,
        .states_count  = n + 2,
        .states        = (const struct parser_state_transition **) states,
        .states_n      = (const size_t *) nstates,

    };

    return ret;
}

static void eq(const uint8_t c, struct action *act) {
    act->type    = 0; // 0 es igual
    act->n       = 1;
    act->data[0] = c;
}

static void neq(const uint8_t c, struct action *act) {
    act->type    = 1; // 1 es not equal
    act->n       = 1;
    act->data[0] = c;
}
