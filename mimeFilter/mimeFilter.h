#ifndef MIMEFILTER_H_
#define MIMEFILTER_H_


struct parser {

    const unsigned  *classes; 

    const unsigned  statesCant;

    const struct transition **states;

    const size_t  *options; //estados para una transicion

    const unsigned beginHere; //estado domde empiezo

    unsigned state; //estado actual

    struct action ac1;

    struct action ac2;
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

const unsigned *parser_no_classes(void);

struct parser create_parser_for_string(const char *string);

static void eq(const uint8_t c, struct action *act);

static void neq(const uint8_t c, struct action *act);


#endif