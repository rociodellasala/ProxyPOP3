#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "mimeList.h"
#include "mimeFilter.h"
#include "mime_type.h"
#include "parser_utils.h"
#include "pop3_multi.h"
#include "mime_chars.h"
#include "mime_msg.h"
#include "frontier.h"
#include "stack.h"


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

    struct parser_definition ctypeParser = parser_init(no_class,  parser_utils_strcmpi("content-type")); 

    struct parser_definiton boundaryParser = parser_init(no_class,  parser_utils_strcmpi("boundary")); 

    struct parser_definition messageParser = parser_init(init_char_class(), mime_message_parser()); 

    struct parser_definiton multiParser =parser_init(no_class, pop3_multi_parser());  

    struct parser mimeTypeParser = parser_init(init_char_class(), mime_type_parser());  //falta este -- cambiarlo!

    //inicializo estructura
	//falta hacer

    struct currentSituation ctx = {
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
            pop3_multi(&ctx, data[i]); 
        }
    } while (n > 0);


    //FALTAN destroys

    parser_destroy(ctx.multi);
    parser_destroy(ctx.msg);
    parser_destroy(ctx.ctype_header);
    parser_utils_strcmpi_destroy(&media_header_def);
    parser_destroy(ctx.mime_type);
    parser_destroy(ctx.boundary);
    parser_utils_strcmpi_destroy(&boundary_def);
    destroy_list(ctx.list);

    while(!stack_is_empty(ctx.boundary_frontier)) {
        struct Frontier *f = stack_pop(ctx.boundary_frontier);
        frontier_destroy(f);
    }
    stack_destroy(ctx.boundary_frontier);

    return 0;

}

/* Delimita una respuesta multi-línea POP3. Se encarga del "byte-stuffing" */
static void pop3_multi(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(ctx->multi, c);
    do {
        //debug("0. multi", pop3_multi_event, e);
        switch (e->type) {
            case POP3_MULTI_BYTE:
                for (int i = 0; i < e->n; i++) {
                    mime_msg(ctx, e->data[i]);
                }
                break;
            case POP3_MULTI_WAIT:
                // nada para hacer mas que esperar
                break;
            case POP3_MULTI_FIN:
                // arrancamos de vuelta
                parser_reset(ctx->msg);
                ctx->msg_content_type_field_detected = NULL;
                break;
        }
        e = e->next;
    } while (e != NULL);
}



const unsigned *parser_no_classes(void) {  
    return classes;
}


/**
 * Procesa un mensaje `tipo-rfc822'.
 * Si reconoce un al field-header-name Content-Type lo interpreta.
 *
 */
static void mime_msg(struct currentSituation *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(ctx->msg, c);

    bool printed = false;
    do {
        //debug("1.   msg", mime_msg_event, e);
        switch (e->type) {
            case MIME_MSG_NAME:
                if (ctx->msg_content_type_field_detected == 0
                    || *ctx->msg_content_type_field_detected) {
                    for (int i = 0; i < e->n; i++) {
                        content_type_header(ctx, e->data[i]);
                    }
                }
                break;
            case MIME_MSG_NAME_END:
                // lo dejamos listo para el próximo header
                parser_reset(ctx->ctype_header);
                break;
            case MIME_MSG_VALUE:
                for (int i = 0; i < e->n; i++) {
                    ctx->buffer[ctx->i++] = e->data[i];
                    if (ctx->i >= CONTENT_TYPE_VALUE_SIZE) {
                        abort();
                    }

                    if (ctx->msg_content_type_field_detected != 0
                        && *ctx->msg_content_type_field_detected) {
                        content_type_value(ctx, e->data[i]);
                    }
                }
                break;
            case MIME_MSG_VALUE_END:
                if (ctx->filtered_msg_detected != 0 && *ctx->filtered_msg_detected) {
                    ctx->replace = true;
                    printf("text/plain\r\n");
                } else {
                    printf("%s\r\n", ctx->buffer);
                    //printed = true;
                }

                ctx->i = 0;
                for (int i = 0; i < CONTENT_TYPE_VALUE_SIZE; i++) {
                    ctx->buffer[i]  = 0;
                }
                end_frontier(stack_peek(ctx->boundary_frontier));
                parser_reset(ctx->mime_type);
                clean_list(ctx->mime_tree);
                parser_reset(ctx->boundary);
                ctx->msg_content_type_field_detected = 0;
                ctx->filtered_msg_detected = &F;
                break;
            case MIME_MSG_BODY:
                if (ctx->replace && !ctx->replaced) {
                    printf("%s\r\n", ctx->filter_msg);
                    ctx->replaced = true;
                } else if (!ctx->replace){
                    putchar(c);
                    printed = true;
                }
                if ((ctx->boundary_detected != 0
                     && *ctx->boundary_detected) || !stack_is_empty(ctx->boundary_frontier)) {
                    for (int i = 0; i < e->n; i++) {
                        boundary_frontier_check(ctx, e->data[i]);
                        check_end_of_frontier(ctx, e->data[i]);
                        if (!printed && ctx->frontier_end_detected != NULL && *ctx->frontier_end_detected) {
                            putchar(c);
                        }
                    }
                }
                break;
            case MIME_MSG_BODY_NEWLINE:
                if (ctx->frontier_detected != 0 && ctx->frontier_end_detected != 0
                    && (*ctx->frontier_end_detected && !*ctx->frontier_detected)) {
                    struct Frontier *f = stack_pop(ctx->boundary_frontier);
                    if (f != NULL) {
                        frontier_destroy(f);
                    }
                }
                if (ctx->frontier_detected != 0 && *ctx->frontier_detected) {
                    ctx->replace = false;
                    ctx->replaced = false;
                    ctx->filtered_msg_detected = &F;
                    ctx->boundary_detected = &F;
                    ctx->frontier_detected = NULL;
                    ctx->frontier_end_detected = NULL;
                    ctx->subtype = NULL;
                    ctx->msg_content_type_field_detected = NULL;
                    parser_reset(ctx->msg);
                    clean_list(ctx->list);
                    parser_reset(ctx->mime_type);
                    parser_reset(ctx->boundary);
                    parser_reset(ctx->ctype_header);
                    frontier_reset(stack_peek(ctx->boundary_frontier));
                }
                if (stack_peek(ctx->boundary_frontier) != NULL) {
                    parser_reset(((struct Frontier *) stack_peek(ctx->boundary_frontier))->frontier_parser);
                    parser_reset(((struct Frontier *) stack_peek(ctx->boundary_frontier))->frontier_end_parser);
                }
                break;
            case MIME_MSG_VALUE_FOLD:
                for (int i = 0; i < e->n; i++) {
                    ctx->buffer[ctx->i++] = e->data[i];
                    if (ctx->i >= CONTENT_TYPE_VALUE_SIZE) {
                        abort();
                    }
                }
                break;
            default:
                break;
        }

        if (should_print(e) && !printed) {
            if ((c == '\r' || c== '\n') && (e->type == MIME_MSG_BODY_NEWLINE || e->type == MIME_MSG_BODY_CR) && ctx->replaced) {
                // nada por hacer
            } else {
                putchar(c);
                printed = true;
            }
        }

        e = e->next;
    } while (e != NULL);

}

static bool T = true;
static bool F = false;


void setContextType(struct ctx *ctx) {
    struct TreeNode *node = ctx->mime_tree->first;
    if (node->event->type == STRING_CMP_EQ) {
        ctx->subtype = node->children;
        return;
    }

    while (node->next != NULL) {
        node = node->next;
        if (node->event->type == STRING_CMP_EQ) {
            ctx->subtype = node->children;
            return;
        }
    }
}

const struct parser_event * parser_feed_type(struct Tree *mime_tree, const uint8_t c) {
    struct TreeNode *node = mime_tree->first;
    const struct parser_event *global_event;
    node->event = parser_feed(node->parser, c);
    global_event = node->event;
    while (node->next != NULL) {
        node = node->next;
        node->event = parser_feed(node->parser, c);
        if (node->event->type == STRING_CMP_EQ) {
            global_event = node->event;
        }
    }
    return global_event;
}

bool global_e = false;

const struct parser_event * parser_feed_subtype(struct TreeNode *node, const uint8_t c) {
    struct parser_event *global_event;

    if (node->wildcard) {
        global_e = true;
        global_event = malloc(sizeof(*global_event));
        if (global_event == NULL) {
            return NULL;
        }
        memset(global_event, 0, sizeof(*global_event));
        global_event->type = STRING_CMP_EQ;
        global_event->next = NULL;
        global_event->n = 1;
        global_event->data[0] = c;
        return global_event;
    }
    global_e = false;
    node->event = parser_feed(node->parser, c);
    global_event = (struct parser_event *) node->event;

    while (node->next != NULL) {
        node = node->next;
        node->event = parser_feed(node->parser, c);
        if (node->event->type == STRING_CMP_EQ) {
            global_event = (struct parser_event *) node->event;
        }
    }
    return global_event;
}

static void check_end_of_frontier(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(
            ((struct Frontier *) stack_peek(ctx->boundary_frontier))->frontier_end_parser, c);
    do {
        //debug("7.Body", parser_utils_strcmpi_event, e);
        switch (e->type) {
            case STRING_CMP_EQ:
                ctx->frontier_end_detected = &T;
                break;
            case STRING_CMP_NEQ:
                ctx->frontier_end_detected = &F;
                break;
        }
        e = e->next;
    } while (e != NULL);
}


static void boundary_frontier_check(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(
            ((struct Frontier *) stack_peek(ctx->boundary_frontier))->frontier_parser, c);
    do {
        //debug("6.Body", parser_utils_strcmpi_event, e);
        switch (e->type) {
            case STRING_CMP_EQ:
                ctx->frontier_detected = &T;
                break;
            case STRING_CMP_NEQ:
                ctx->frontier_detected = &F;
        }
        e = e->next;
    } while (e != NULL);
}

static void store_boundary_parameter(struct ctx *ctx, const uint8_t c) {
    add_character(stack_peek(ctx->boundary_frontier), c);
}


static void parameter_boundary(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(ctx->boundary, c);
    do {
        //debug("5.Boundary", parser_utils_strcmpi_event, e);
        switch (e->type) {
            case STRING_CMP_EQ:
                ctx->boundary_detected = &T;
                break;
            case STRING_CMP_NEQ:
                ctx->boundary_detected = &F;
                break;
        }
        e = e->next;
    } while (e != NULL);
}

static void content_type_subtype(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed_subtype(ctx->subtype, c);
    if (e == NULL) {
        //TODO destro ctx
        return;
    }
    do {
        //debug("4.subtype", parser_utils_strcmpi_event, e);
        switch (e->type) {
            case STRING_CMP_EQ:
                ctx->filtered_msg_detected = &T;
                break;
            case STRING_CMP_NEQ:
                ctx->filtered_msg_detected = &F;
                break;
        }

        const struct parser_event *next = e->next;
        if (global_e) {
            free((void *) e);
            global_e = false;
        }
        e = next;
    } while (e != NULL);
}


static void content_type_type(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed_type(ctx->mime_tree, c);
    do {
        //debug("4.type", parser_utils_strcmpi_event, e);
        switch (e->type) {
            case STRING_CMP_EQ:
                ctx->filtered_msg_detected = &T;
                break;
            case STRING_CMP_NEQ:
                ctx->filtered_msg_detected = &F;
                break;
        }
        e = e->next;
    } while (e != NULL);
}

static void content_type_value(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(ctx->mime_type, c);
    do {
        //debug("3.typeval", mime_type_event, e);
        switch (e->type) {
            case MIME_TYPE_TYPE:
                if (ctx->filtered_msg_detected != 0
                    || *ctx->filtered_msg_detected)
                    for (int i = 0; i < e->n; i++) {
                        content_type_type(ctx, e->data[i]);
                    }
                break;
            case MIME_TYPE_SUBTYPE:
                if (ctx->filtered_msg_detected != 0
                    && *ctx->filtered_msg_detected)
                    content_type_subtype(ctx, c);
                break;
            case MIME_TYPE_TYPE_END:
                if (ctx->filtered_msg_detected != 0
                    || *ctx->filtered_msg_detected) {
                    setContextType(ctx);
                }
                break;
            case MIME_PARAMETER:
                parameter_boundary(ctx, c);
                break;
            case MIME_FRONTIER_START:
                if (ctx->boundary_detected != 0
                    && *ctx->boundary_detected) {
                    struct Frontier *f = frontier_init();
                    if (f == NULL) {
                        abort();
                    }
                    stack_push(ctx->boundary_frontier, f);
                }
                break;
            case MIME_FRONTIER:
                if (ctx->boundary_detected != 0
                    && *ctx->boundary_detected) {
                    for (int i = 0; i < e->n; i++) {
                        store_boundary_parameter(ctx, e->data[i]);
                    }
                }
                break;
        }
        e = e->next;
    } while (e != NULL);
}


/* Detecta si un header-field-name equivale a Content-Type.
 * Deja el valor en `ctx->msg_content_type_field_detected'. Tres valores
 * posibles: NULL (no tenemos información suficiente todavia, por ejemplo
 * viene diciendo Conten), true si matchea, false si no matchea.
 */
static void content_type_header(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(ctx->ctype_header, c);
    do {
        //debug("2.typehr", parser_utils_strcmpi_event, e);
        switch (e->type) {
            case STRING_CMP_EQ:
                ctx->msg_content_type_field_detected = &T;
                break;
            case STRING_CMP_NEQ:
                ctx->msg_content_type_field_detected = &F;
                break;
        }
        e = e->next;
    } while (e != NULL);
}

bool should_print(const struct parser_event *e) {
    return e->type != MIME_MSG_BODY && e->type != MIME_MSG_VALUE && e->type != MIME_MSG_VALUE_END
           && e->type != MIME_MSG_WAIT && e->type != MIME_MSG_VALUE_FOLD;
}
