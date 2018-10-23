#ifndef MIMEFILTER_H_
#define MIMEFILTER_H_

#include <stdint.h>
#include "mimeList.h"
#define CONTENT_TYPE_VALUE_SIZE 2048


struct ctx {
    /* delimitador respuesta multi-línea POP3 */
    struct parser *multi;
    /* delimitador mensaje "tipo-rfc 822" */
    struct parser *msg;
    /* detector de field-name "Content-Type" */
    struct parser *ctype_header;
    /* parser de mime type "tipo rfc 2045" */
    struct parser *mime_type;
    /* estructura que contiene los tipos filtrados*/
    struct List *mime_list;
    /* estructura que contiene los subtipos del tipo encontrado */
    struct subtype_node *subtype;
    /* detector de parametro boundary en un header Content-Type */
    struct parser *boundary;
    /* stack de frontiers que permite tener boundaries anidados */
    struct stack *boundary_frontier;

    char * filter_msg;
    bool replace, replaced;

    /* ¿hemos detectado si el field-name que estamos procesando refiere
     * a Content-Type?. Utilizando dentro msg para los field-name.
     */
    bool *msg_content_type_field_detected;
    bool *frontier_end_detected;
    bool *frontier_detected;
    bool *filtered_msg_detected;
    bool *boundary_detected;

    // content type value
    char buffer[CONTENT_TYPE_VALUE_SIZE];
    unsigned i;
};

const unsigned *parser_no_classes(void);

static void pop3_multi(struct ctx *ctx, const uint8_t c);

static void mime_msg(struct ctx *ctx, const uint8_t c);

void setContextType(struct ctx *ctx) ;

const struct parser_event * parser_feed_subtype(struct subtype_node *node, const uint8_t c);

static void check_end_of_frontier(struct ctx *ctx, const uint8_t c);

static void boundary_frontier_check(struct ctx *ctx, const uint8_t c);

static void store_boundary_parameter(struct ctx *ctx, const uint8_t c);

static void parameter_boundary(struct ctx *ctx, const uint8_t c);

static void content_type_subtype(struct ctx *ctx, const uint8_t c);

static void content_type_type(struct ctx *ctx, const uint8_t c);

static void content_type_value(struct ctx *ctx, const uint8_t c);

static void content_type_header(struct ctx *ctx, const uint8_t c);

bool should_print(const struct parser_event *e);





#endif