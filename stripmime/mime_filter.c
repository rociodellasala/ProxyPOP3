#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "mime_list.h"
#include "mime_filter.h"
#include "mime_type.h"
#include "parser_utils.h"
#include "parser_creator.h"
#include "multi_pop3_parser.h"
#include "mime_chars.h"
#include "mime_msg.h"
#include "delimiter.h"
#include "stack.h"


//variables de entorno del manual pop3filter.8
#define FILTER_MEDIAS 	"FILTER_MEDIAS"
#define FILTER_MSG 		"FILTER_MSG"
#define ANY (1 << 9) 

bool is_wildcard = false;
bool new_boundary_push = true;
bool first_attempt = true;
static bool T = true;
static bool F = false;

static void detect_delimiter_ending(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(
            ((struct delimiter_st *) stack_peek(ctx->boundary_delimiter))->delimiter_end_parser, c);
    do {
        switch (e->type) {
            case STRING_CMP_EQ:
                ctx->delimiter_end_detected = &T;
                break;
            case STRING_CMP_NEQ:
                ctx->delimiter_end_detected = &F;
                break;
        }
        e = e->next;
    } while (e != NULL);
}


static void boundary_delimiter_detection(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(
            ((struct delimiter_st *) stack_peek(ctx->boundary_delimiter))->delimiter_parser, c);
    do {
        switch (e->type) {
            case STRING_CMP_EQ:
                ctx->delimiter_detected = &T;
                break;
            case STRING_CMP_NEQ:
                ctx->delimiter_detected = &F;
        }
        e = e->next;
    } while (e != NULL);
}

static void boundary_analizer(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(ctx->boundary, c);
    do {
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
    const struct parser_event *e = feed_subtypes(ctx->subtype, c);
    if (e == NULL) {
        // BORRAR ESTO CAMBIARLO DESTRUIR CTX
        return;
    }
    do {
        switch (e->type) {
            case STRING_CMP_EQ:
                ctx->to_be_censored = &T;
                break;
            case STRING_CMP_NEQ:
                ctx->to_be_censored = &F;
                break;
        }

        const struct parser_event *next = e->next;
        if (is_wildcard) {
            free((void *) e);
            is_wildcard = false;
        }
        e = next;
    } while (e != NULL);
}


static void content_type_type(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = feed_types(ctx->mime_list, c);
    do {
        switch (e->type) {
            case STRING_CMP_EQ:
                ctx->to_be_censored = &T;
                break;
            case STRING_CMP_NEQ:
                ctx->to_be_censored = &F;
                break;
        }
        e = e->next;
    } while (e != NULL);
}


static void content_type_value(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(ctx->mime_type, c);
    do {
        switch (e->type) {
            case MIME_TYPE_TYPE:
                if (ctx->to_be_censored != 0 || *ctx->to_be_censored)
                    for (int i = 0; i < e->n; i++) {
                        content_type_type(ctx, e->data[i]);
                    }
                break;
            case MIME_TYPE_SUBTYPE:
                if (ctx->to_be_censored != 0 && *ctx->to_be_censored)
                    content_type_subtype(ctx, c);
                break;
            case MIME_TYPE_TYPE_END:
                if (ctx->to_be_censored != 0 || *ctx->to_be_censored) {              
                    context_setter(ctx);
                }
                break;
            case MIME_PARAMETER:
                boundary_analizer(ctx, c);
                break;
            case MIME_DELIMITER_START:
                if ((ctx->boundary_detected != 0 && *ctx->boundary_detected)) {
                    struct delimiter_st *dlm = delimiter_init();
                    if (dlm == NULL) {
                        abort(); //mandar esto adentro del if de abajo. y compara como hice con multipar/alternatvie tmbn con mixed
                                // y que new_boundary_push empiece en false!
                    }
                    if(new_boundary_push == true){
                        stack_push(ctx->boundary_delimiter, dlm);
                        new_boundary_push = false;
                    }
                    
                    

                }
                
                break;
            case MIME_DELIMITER:
                if (ctx->boundary_detected != 0 && *ctx->boundary_detected) {
                    for (int i = 0; i < e->n; i++) {

                        extend(e->data[i], stack_peek(ctx->boundary_delimiter));
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

/* Detecta si un header-field-name equivale a Content-Transfer-Encoding.
 * Deja el valor en `ctx->msg_transfer_encoding_detected'. Tres valores
 * posibles: NULL (no tenemos información suficiente todavia, por ejemplo
 * viene diciendo Conten), true si matchea, false si no matchea.
 */
static void transfer_encoding_header(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(ctx->transfer_encoding_header, c);
    do {

        switch (e->type) {
            case STRING_CMP_EQ:
                ctx->msg_transfer_encoding_detected = &T;
                break;
            case STRING_CMP_NEQ:
                ctx->msg_transfer_encoding_detected = &F;
                break;
        }
        e = e->next;
    } while (e != NULL);
}

/* Detecta si un header-field-name equivale a Content-Disposition.
 * Deja el valor en `ctx->msg_disposition_detected'. Tres valores
 * posibles: NULL (no tenemos información suficiente todavia, por ejemplo
 * viene diciendo Conten), true si matchea, false si no matchea.
 */
static void disposition_header(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(ctx->disposition_header, c);
    do {

        switch (e->type) {
            case STRING_CMP_EQ:
                ctx->msg_disposition_detected = &T;
                break;
            case STRING_CMP_NEQ:
                ctx->msg_disposition_detected = &F;
                break;
        }
        e = e->next;
    } while (e != NULL);
}

/**
 * Procesa un mensaje `tipo-rfc822'.
 * Si reconoce un al field-header-name Content-Type lo interpreta.
 *
 */
static void mime_msg(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(ctx->msg, c);

    bool printed = false;
    do {


        switch (e->type) {
            case MIME_MSG_NAME:
                if (ctx->msg_content_type_field_detected == 0 || *ctx->msg_content_type_field_detected) {
                    for (int i = 0; i < e->n; i++) {
                        content_type_header(ctx, e->data[i]);
                    }

                }
                if(ctx->msg_transfer_encoding_detected == 0 || *ctx->msg_transfer_encoding_detected){
                    for (int i = 0; i < e->n; i++) {
                        transfer_encoding_header(ctx, e->data[i]);
                    }
                }
                if(ctx->msg_disposition_detected == 0 || *ctx->msg_disposition_detected){
                    for (int i = 0; i < e->n; i++) {
                        disposition_header(ctx, e->data[i]);
                    }
                }
                break;
            case MIME_MSG_NAME_END:
                parser_reset(ctx->ctype_header);
                parser_reset(ctx->transfer_encoding_header);
                parser_reset(ctx->disposition_header);
                break;

            case MIME_MSG_VALUE:
                //EN NUESTROS LINUX SE QUEDA SIEMPRE EN ESTE CASO Y NUNCA PASA A MIME_MSG_VALUE_END
                // EN PAMPERO FUNCIONA BIEN
                for (int i = 0; i < e->n; i++) {
                    ctx->buffer[ctx->i++] = e->data[i];

                    if (ctx->i >= MAX) {
                        abort(); //eventualmente termina abortando
                    }
                    if (ctx->msg_content_type_field_detected != 0
                        && *ctx->msg_content_type_field_detected) {
                        content_type_value(ctx, e->data[i]);

                    }
                }
                break;
            case MIME_MSG_VALUE_END:
                //printf("IM DONE\n");
                ;
                char * testing = strstr(ctx->buffer, "message");
                if(testing != NULL){
                    ctx->attachment = true;
                }

                char * testing2 = strstr(ctx->buffer, "multipart/alternative");
                if(testing2 != NULL){
                    if(first_attempt == true){
                        first_attempt = false;
                    }else{
                        new_boundary_push = true;
                    }                 
                }else{
                    char * testing3 = strstr(ctx->buffer, "multipart/mixed");
                    if(testing3 != NULL){
                        if(first_attempt == true){
                            first_attempt = false;
                        }else{
                            new_boundary_push = true;
                        }   
                    }
                }
                
                

                if (ctx->to_be_censored != 0 && *ctx->to_be_censored) {
                    ctx->replace = true;
                    printf("text/plain\r\n");
                }else if(ctx->replace && ctx->msg_transfer_encoding_detected != 0 && *ctx->msg_transfer_encoding_detected){
                    printf(" 7bit\n");
                } else if(ctx->replace && ctx->msg_disposition_detected != 0 && *ctx->msg_disposition_detected){
                    printf(" inline\n");
                }else {
                    printf("%s\r\n", ctx->buffer);
                }

                ctx->i = 0;
                for (int i = 0; i < MAX; i++) {
                    ctx->buffer[i]  = 0;
                }
                close_delimiter(stack_peek(ctx->boundary_delimiter));
                parser_reset(ctx->mime_type);
                clean_list(ctx->mime_list);
                parser_reset(ctx->boundary);
                parser_reset(ctx->transfer_encoding_header);
                parser_reset(ctx->disposition_header);
                ctx->msg_transfer_encoding_detected = 0;
                ctx->msg_content_type_field_detected = 0;
                ctx->msg_disposition_detected = 0;
                ctx->to_be_censored = &F;
                break;
            case MIME_MSG_BODY:
                    if (ctx->replace && !ctx->replaced) {
                        printf("%s\r\n", ctx->replacement_text);
                        ctx->replaced = true;
                    } else if (!ctx->replace){
                        putchar(c);
                        printed = true;
                    }
                    if ((ctx->boundary_detected != 0
                         && *ctx->boundary_detected) || !(ctx->boundary_delimiter->size == 0)) {
                        for (int i = 0; i < e->n; i++) {
                            boundary_delimiter_detection(ctx, e->data[i]);
                            detect_delimiter_ending(ctx, e->data[i]);
                            if (!printed && ctx->delimiter_end_detected != NULL && *ctx->delimiter_end_detected) {
                                putchar(c);
                            }
                        }
                    }  

                break;
            case MIME_MSG_BODY_NEWLINE:
               
                if(ctx->attachment == true){
                    ctx->attachment = false;
                    parser_reset(ctx->msg);               
                    parser_reset(ctx->multi);
                    struct delimiter_st *aux = delimiter_init();
                    if (aux == NULL) {
                        abort();
                    }
                    stack_push(ctx->boundary_delimiter, aux);
                    break;
                }
                if (ctx->delimiter_detected != 0 && ctx->delimiter_end_detected != 0
                    && (*ctx->delimiter_end_detected && !*ctx->delimiter_detected)) {
                    struct delimiter_st *dlm = stack_pop(ctx->boundary_delimiter);
                    if (dlm != NULL) {
                        delimiter_destroy(dlm);

                    }
                }
                if (ctx->delimiter_detected != 0 && *ctx->delimiter_detected) {
                    ctx->replace = false;
                    ctx->replaced = false;
                    ctx->to_be_censored = &F;
                    ctx->boundary_detected = &F;
                    ctx->delimiter_detected = NULL;
                    ctx->delimiter_end_detected = NULL;
                    ctx->subtype = NULL;
                    ctx->msg_content_type_field_detected = NULL;
                    parser_reset(ctx->msg);
                    clean_list(ctx->mime_list);
                    parser_reset(ctx->mime_type);
                    parser_reset(ctx->boundary);
                    parser_reset(ctx->ctype_header);
                    parser_reset(ctx->transfer_encoding_header);
                    ctx->msg_transfer_encoding_detected = NULL;
                    parser_reset(ctx->disposition_header);
                    ctx->msg_disposition_detected = NULL;
                    delimiter_reset(stack_peek(ctx->boundary_delimiter));
                }
                if (stack_peek(ctx->boundary_delimiter) != NULL) {
                    parser_reset(((struct delimiter_st *) stack_peek(ctx->boundary_delimiter))->delimiter_parser);
                    parser_reset(((struct delimiter_st *) stack_peek(ctx->boundary_delimiter))->delimiter_end_parser);
                }
                break;
            case MIME_MSG_VALUE_FOLD:
                for (int i = 0; i < e->n; i++) {

                    //e->data[i];
                    ctx->buffer[ctx->i++] = e->data[i]; 
                    if (ctx->i >= MAX) {
                        abort();
                    }
                }
                break;
            default:
                break;
        }

        if (e->type != MIME_MSG_BODY && e->type != MIME_MSG_VALUE && e->type != MIME_MSG_VALUE_END
           && e->type != MIME_MSG_WAIT && e->type != MIME_MSG_VALUE_FOLD && !printed) {
            if ((c == '\r' || c== '\n') && (e->type == MIME_MSG_BODY_NEWLINE || e->type == MIME_MSG_BODY_CR) && ctx->replaced) {
                // nothing
            } else {
                putchar(c);
                printed = true;
            }
        }

        e = e->next;
    } while (e != NULL);

}

/* Delimita una respuesta multi-línea POP3. Se encarga del "byte-stuffing" */
static void pop3_multi(struct ctx *ctx, const uint8_t c) {
    const struct parser_event *e = parser_feed(ctx->multi, c);
    do {
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
                ctx->msg_transfer_encoding_detected = NULL;
                break;
        }
        e = e->next;
    } while (e != NULL);

}


int main(int argc, char ** argv) {

	/*
	* getenv(const char *name) searches for the environment string 
	* pointed to by name and returns the associated value to the string. 
	*/

	char* flm = getenv("FILTER_MEDIAS");

	struct List *list = create_list();

    if(list == NULL){
        return -1;
    }

	if(flm == NULL){
		printf("Unable to get filter medias\n");
		free(list);
		return -1;
	}
	
	
	char* medias = malloc(strlen(flm) + 1);
	
	if(medias == NULL){
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

	while(current != NULL){
		char *aux = malloc(strlen(current) + 1);
		if(aux == NULL){
			free(aux);
			return -1;
		}
		strcpy(aux, current);
	
		/* getting type */
		
		mime = strtok_r(aux, slash, &context_b);
		if(mime == NULL){
            free(aux);
			return -1;
		}

		char *type = malloc(strlen(mime) + 1);
		if(type == NULL){
            free(type);
			return -1;
		}
		strcpy(type, mime);
		/*getting subtype*/

		mime = strtok_r(NULL, slash, &context_b);
		if(mime == NULL){

			return -1;
		}

		char *subtype = malloc(strlen(mime) + 1);

		if(subtype == NULL){
            free(subtype);
			return -1;
		}

		strcpy(subtype, mime);
		int addition = add_new(type, subtype, list);
		if(addition != -1){
			//printf("Node addition made\n");
		}

		free(aux);
        
		current = strtok_r(NULL, comma, &context);

	}
	
    free(medias);
	
	char *message = getenv(FILTER_MSG);

    if (message == NULL) {
        message = "Parte reemplazada.";
    }
	

	const unsigned int *no_class = parser_no_classes(); 

    struct parser_definition media_header_def= parser_utils_strcmpi("content-type");

    struct parser_definition boundary_def = parser_utils_strcmpi("boundary");

    struct parser_definition encoding_def = parser_utils_strcmpi("content-transfer-encoding");

    struct parser_definition disposition_def = parser_utils_strcmpi("content-disposition");

    struct parser *ctypeParser = parser_init(no_class,  &media_header_def); 

    struct parser *boundaryParser = parser_init(no_class,  &boundary_def); 

    struct parser *transferParser = parser_init(no_class,  &encoding_def); 

    struct parser *dispositionParser = parser_init(no_class,  &disposition_def); 

    struct parser *messageParser = parser_init(init_char_class(), mime_message_parser()); 

    struct parser *multiParser =parser_init(no_class, pop3_multi_parser());  

    struct parser *mimeTypeParser = parser_init(init_char_class(), mime_type_parser());  

    struct ctx ctx = {
            .multi                  = multiParser,
            .msg                    = messageParser,
            .ctype_header           = ctypeParser,
            .transfer_encoding_header = transferParser,
            .disposition_header     = dispositionParser,
            .mime_type              = mimeTypeParser,
            .boundary               = boundaryParser,
            .mime_list              = list,
            .boundary_delimiter      = stack_init(),
            .to_be_censored  = NULL,
            .boundary_detected      = NULL,
            .delimiter_end_detected  = NULL,
            .delimiter_detected      = NULL,
            .replacement_text       = message,
            .replace                = false,
            .replaced               = false,
            .buffer                 = {0},
            .i                      = 0,
            .attachment = false,
    };


	uint8_t data[4096];

    ssize_t n;
    int fd = STDIN_FILENO;
    do {

        n = read(fd, data, sizeof(data));
        //printf("ok n es %d\n", n);
        for (ssize_t i = 0; i < n; i++) {   

             pop3_multi(&ctx, data[i]); 
        }

    } while (n > 0);



    parser_destroy(ctx.multi);
    parser_destroy(ctx.msg);
    parser_destroy(ctx.ctype_header);
    parser_utils_strcmpi_destroy(&media_header_def);
    parser_destroy(ctx.mime_type);
    parser_destroy(ctx.boundary);
    parser_utils_strcmpi_destroy(&boundary_def);
    destroy_list(ctx.mime_list);
    parser_destroy(ctx.disposition_header);
    parser_destroy(ctx.transfer_encoding_header);
    parser_utils_strcmpi_destroy(&disposition_def);
    parser_utils_strcmpi_destroy(&encoding_def);

    while(!(ctx.boundary_delimiter->size == 0)) {
        struct delimiter_st *dlm = stack_pop(ctx.boundary_delimiter);
        delimiter_destroy(dlm);
    }
    stack_destroy(ctx.boundary_delimiter);

    return 0;

}


void context_setter(struct ctx *ctx) {
    struct type_node *node = ctx->mime_list->first;
    if (node->event->type == STRING_CMP_EQ) {
        ctx->subtype = node->subtypes;
        return;
    }

    while (node->next != NULL) {
        node = node->next;
        if (node->event->type == STRING_CMP_EQ) {
            ctx->subtype = node->subtypes;
            return;
        }
    }
}

const struct parser_event * feed_types(struct List *mime_list, const uint8_t c) {

    struct type_node *node = mime_list->first;
    const struct parser_event *curr;
    node->event = parser_feed(node->parser, c);
    curr = node->event;

    while (node->next != NULL) {
        node = node->next;
        node->event = parser_feed(node->parser, c);
        if (node->event->type == STRING_CMP_EQ) {
            curr = node->event;
        }
    }

    return curr;
}



const struct parser_event * feed_subtypes(struct subtype_node *node, const uint8_t c) {

    struct parser_event *evt;

    if (node->wildcard) {
        is_wildcard = true;
        evt = malloc(sizeof(*evt));
        if (evt == NULL) {
            return NULL;
        }
        memset(evt, 0, sizeof(*evt));
        evt->type = STRING_CMP_EQ;
        evt->next = NULL;
        evt->n = 1;
        evt->data[0] = c;
        return evt;
    }
    is_wildcard = false;
    node->event = parser_feed(node->parser, c);
    evt = (struct parser_event *) node->event;

    while (node->next != NULL) {
        node = node->next;
        node->event = parser_feed(node->parser, c);
        if (node->event->type == STRING_CMP_EQ) {
            evt = (struct parser_event *) node->event;
        }
    }
    return evt;
}



















