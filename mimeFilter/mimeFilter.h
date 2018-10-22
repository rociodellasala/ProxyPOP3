#ifndef MIMEFILTER_H_
#define MIMEFILTER_H_


struct parser {

    const unsigned  *classes; 

	/** definición de estados */
    const struct parser_definition *def;

    unsigned state; //estado actual

    struct action ac1;

    struct action ac2;
};


/** declaración completa de una máquina de estados */
struct parser_definition {
    /** cantidad de estados */
    const unsigned                         states_count;
    /** por cada estado, sus transiciones */
    const struct parser_state_transition **states;
    /** cantidad de estados por transición */
    const size_t                          *states_n;

    /** estado inicial */
    const unsigned                         start_state;
};


/**
 * Evento que retorna el parser.
 * Cada tipo de evento tendrá sus reglas en relación a data.
 */
struct action {
    /** tipo de evento */
    unsigned type;
    /** caracteres asociados al evento */
    uint8_t  data[3];
    /** cantidad de datos en el buffer `data' */
    uint8_t  n;

    /** lista de eventos: si es diferente de null ocurrieron varios eventos */
    struct action *next;
};

/** describe una transición entre estados  */
struct transition {
    /* condición: un caracter o una clase de caracter. Por ej: '\r' */
    int       received;
    /** descriptor del estado destino cuando se cumple la condición */
    unsigned  go;
    /** acción 1 que se ejecuta cuando la condición es verdadera. requerida. */
    void    (*act1)(struct action *ret, const uint8_t c);
    /** otra acción opcional */
    void    (*act2)(struct action *ret, const uint8_t c);
};


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


#endif