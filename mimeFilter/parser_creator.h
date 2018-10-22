#ifndef PARSER_CREATOR_H_
#define PARSER_CREATOR_H_

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

struct parser *parser_init    (const unsigned *classes, const struct parser_definition *def);

void parser_destroy(struct parser *p);

struct parser_definition create_parser_def_message();

struct parser_definition create_parser_def_multi_pop();

struct parser_definition create_mimeType_def_parser();

#endif