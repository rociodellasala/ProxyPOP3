#ifndef MIME_FILTER_H_
#define MIME_FILTER_H_

#include <stdint.h>
#include "mime_list.h"
#define MAX 2048


struct ctx {

    /* delimitador respuesta multi-línea POP3 */
    struct parser *multi;

    /* delimitador mensaje "tipo-rfc 822" */
    struct parser *msg;

    /* detector de field-name "Content-Type" */
    struct parser *ctype_header;

    /* detector de field-name "Content-Transfer-Encoding" */
    struct parser *transfer_encoding_header;

    /* detector de field-name "Content-Disposition" */
    struct parser *disposition_header;

    /* parser mime type "tipo-rfc 2045" */
    struct parser *mime_type;

    /* lista de tipos a filtrar*/
    struct List *mime_list;

    /* lista de subtipos de cierto tipo */
    struct subtype_node *subtype;

    /* detecta de boundary en un header Content-Type */
    struct parser *boundary;

    /* stack de delimitadores para anidaciones de boundaries */
    struct stack *boundary_delimiter;

    /* texto de reemplazo */
    char * replacement_text;

    /* booleanos auxiliares*/
    bool replace;
    bool replaced;

    /* detecamos un content transfer encoding */
    bool *msg_transfer_encoding_detected;

    /* detecamos un content disposition */
    bool *msg_disposition_detected;

    /* ¿hemos detectado si el field-name que estamos procesando refiere
     * a Content-Type?. Utilizando dentro msg para los field-name.
     */
    bool *msg_content_type_field_detected;

    /* detectamos fin de un delimitador */
    bool *delimiter_end_detected;

    /* detectamos un delimitador */
    bool *delimiter_detected;

    /* detectamos que debemos reemplazar el texto */
    bool *to_be_censored;

    /* detectamos un boundary */
    bool *boundary_detected;

    /* tenemos un mail anidado*/
    bool attachment;

    /*  buffer para analizar un content type */
    char buffer[MAX];
    unsigned i;
};

const unsigned *parser_no_classes(void);

void context_setter(struct ctx *ctx) ;

const struct parser_event * feed_subtypes(struct subtype_node *node, const uint8_t c);

const struct parser_event * feed_types(struct List *mime_list, const uint8_t c);

#endif