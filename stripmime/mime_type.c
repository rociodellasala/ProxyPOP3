#include "include/parser_creator.h"
#include "include/mime_chars.h"
#include "include/mime_type.h"

/**
 *
 *	MIME type:
 *
 *	type/subtype
 *
 */

enum state {
	TYPE0,
    TYPE,
    SUBTYPE,
    PARAMETER,
    DELIMITER_START,
    DELIMITER,
    DELIMITER_END,
    ERROR,
};

///////////////////////////////////////////////////////////////////////////////
// Acciones

static void type(struct parser_event *ret, const uint8_t c) {
    ret->type    = MIME_TYPE_TYPE;
    ret->n       = 1;
    ret->data[0] = c;
}

static void type_end(struct parser_event *ret, const uint8_t c) {
    ret->type    = MIME_TYPE_TYPE_END;
    ret->n       = 1;
	ret->data[0] = '/';
}

static void subtype(struct parser_event *ret, const uint8_t c) {
    ret->type    = MIME_TYPE_SUBTYPE;
    ret->n       = 1;
    ret->data[0] = c;
}

static void unexpected(struct parser_event *ret, const uint8_t c) {
	ret->type    = MIME_TYPE_UNEXPECTED;
    ret->n       = 1;
    ret->data[0] = c;	
}

static void parameter_start(struct parser_event *ret, const uint8_t c){
    ret->type    = MIME_PARAMETER_START;
    ret->n       = 1;
    ret->data[0] = ';';
}

static void parameter(struct parser_event *ret, const uint8_t c){
    ret->type   = MIME_PARAMETER;
    ret->n      = 1;
    ret->data[0]= c;
}

static void boundary_end(struct parser_event *ret, const uint8_t c){
    ret->type   = MIME_BOUNDARY_END;
    ret->n      = 1;
    ret->data[0]= '=';
}


static void delimiter_start(struct parser_event* ret, const uint8_t c){
    ret->type   = MIME_DELIMITER_START;
    ret->n      = 1;
    ret->data[0]= '\"';
}

static void delimiter(struct parser_event *ret, const uint8_t c){
    ret->type   = MIME_DELIMITER;
    ret->n      = 1;
    ret->data[0]= c;
}

static void delimiter_end(struct parser_event *ret, const uint8_t c){
    ret->type = MIME_DELIMITER_END;
    ret->n    = 1;
    ret->data[0] = c;
}


///////////////////////////////////////////////////////////////////////////////
// Transiciones

static const struct parser_state_transition ST_TYPE0[] =  {
    {.when = '/',        				.dest = ERROR,         		.act1 = unexpected,	},
    {.when = TOKEN_LWSP,   				.dest = ERROR,         		.act1 = unexpected,	},
    {.when = TOKEN_REST_NAME_FIRST,		.dest = TYPE,				.act1 = type,		},
    {.when = ANY, 						.dest = ERROR, 				.act1 = unexpected,	}
};

static const struct parser_state_transition ST_TYPE[] =  {
    {.when = '/',        				.dest = SUBTYPE,         	.act1 = type_end,	},
    {.when = TOKEN_LWSP,   				.dest = ERROR,          	.act1 = unexpected,	},
    {.when = TOKEN_REST_NAME_CHARS,		.dest = TYPE,				.act1 = type,		},
    {.when = ANY, 						.dest = ERROR, 				.act1 = unexpected,	}
};

static const struct parser_state_transition ST_SUBTYPE[] =  {
	{.when = TOKEN_LWSP,   				.dest = ERROR,          	           .act1 = unexpected,	      },
    {.when = ';',                       .dest = PARAMETER,                     .act1 = parameter_start,   },
    {.when = TOKEN_REST_NAME_CHARS,		.dest = SUBTYPE,			           .act1 = subtype,	          },
    {.when = ANY, 						.dest = ERROR, 				           .act1 = unexpected,	      }
};

static const struct parser_state_transition ST_PARAMETER[] = {
    {.when = TOKEN_LWSP,                .dest = ERROR,               .act1 = unexpected,},
    {.when = TOKEN_REST_NAME_CHARS,     .dest = PARAMETER,           .act1 = parameter, },
    {.when = '=',                       .dest = DELIMITER_START,      .act1 = boundary_end,},
    {.when = ANY,                       .dest = ERROR,               .act1 = unexpected,}
};

static const struct parser_state_transition ST_DELIMITER_START[] = {
        {.when = '\"',      .dest = DELIMITER,           .act1 = delimiter_start},
        {.when = ANY,       .dest = ERROR,              .act1 = unexpected,},
};

static const struct parser_state_transition ST_DELIMITER[] = {
        {.when = TOKEN_REST_NAME_CHARS,     .dest = DELIMITER,         .act1 = delimiter,     },
        {.when = TOKEN_BCHARS_NOSPACE,      .dest = DELIMITER,         .act1 = delimiter,     },
        {.when = '\"',                      .dest = DELIMITER_END,     .act1 = delimiter_end, },
        {.when = ANY,                       .dest = ERROR,            .act1 = unexpected,   },
};

static const struct parser_state_transition ST_DELIMITER_END[] = {
        {.when = ANY,       .dest = DELIMITER_END,       .act1 = delimiter_end},
};

static const struct parser_state_transition ST_ERROR[] =  {
    {.when = ANY,        				.dest = ERROR,         		.act1 = unexpected,	},
};


///////////////////////////////////////////////////////////////////////////////
// Declaración formal

static const struct parser_state_transition *states [] = {
    ST_TYPE0,
    ST_TYPE,
    ST_SUBTYPE,
    ST_PARAMETER,
    ST_DELIMITER_START,
    ST_DELIMITER,
    ST_DELIMITER_END,
    ST_ERROR,
};

#define N(x) (sizeof(x)/sizeof((x)[0]))

static const size_t states_n [] = {
    N(ST_TYPE0),
    N(ST_TYPE),
    N(ST_SUBTYPE),
    N(ST_PARAMETER),
    N(ST_DELIMITER_START),
    N(ST_DELIMITER),
    N(ST_DELIMITER_END),
    N(ST_ERROR),
};

static struct parser_definition definition = {
    .states_count = N(states),
    .states       = states,
    .states_n     = states_n,
    .start_state  = TYPE0,
};

const struct parser_definition * 
mime_type_parser(void) {
    return &definition;
}


const char *
mime_type_event(enum mime_type_event_type type) {
    const char *ret = NULL;

    switch(type) {
        case MIME_TYPE_TYPE:
            ret = "type(c)";
            break;
        case MIME_TYPE_TYPE_END:
            ret = "type_end('/')";
            break;
        case MIME_TYPE_SUBTYPE:
            ret = "subtype(c)";
            break;
        case MIME_PARAMETER_START:
            ret = "parameter_start(';')";
            break;
        case MIME_PARAMETER:
            ret = "parameter(c)";
            break;
        case MIME_BOUNDARY_END:
            ret = "boundary_end(c)";
            break;
        case MIME_DELIMITER_START:
            ret = "delimiter_start(c)";
            break;
        case MIME_DELIMITER:
            ret = "delimiter(c)";
            break;
        case MIME_DELIMITER_END:
            ret = "delimiter_end(c)";
            break;
        case MIME_TYPE_UNEXPECTED:
            ret = "unexpected(c)";
            break;
    }
    return ret;
}
