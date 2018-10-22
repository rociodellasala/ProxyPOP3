#ifndef MIMEFILTER_H_
#define MIMEFILTER_H_





struct currentSituation {
    /* delimitador respuesta multi-línea POP3 */
    struct parser *multi;
    /* delimitador mensaje "tipo-rfc 822" */
    struct parser *msg;
    /* detector de field-name "Content-Type" */
    struct parser *ctype_header;
    /* parser de mime type "tipo rfc 2045" */
    struct parser *mime_type;
    /* estructura que contiene los tipos filtrados*/
    struct Tree *mime_tree;
    /* estructura que contiene los subtipos del tipo encontrado */
    struct TreeNode *subtype;
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

struct parser create_parser_for_string(const char *string);

static void eq(const uint8_t c, struct action *act);

static void neq(const uint8_t c, struct action *act);

static void pop3_multi(struct ctx *ctx, const uint8_t c);


#endif