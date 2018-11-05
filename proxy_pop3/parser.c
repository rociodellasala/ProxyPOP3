#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "include/parser.h"

struct parser {
    const unsigned *                    classes;
    const struct parser_definition *    def;

    unsigned                            state;

    struct parser_event                 e1;
    struct parser_event                 e2;
};

struct parser * parser_init(const unsigned * classes, const struct parser_definition * def) {
    struct parser * ret = malloc(sizeof(*ret));

    if (ret != NULL) {
        memset(ret, 0, sizeof(*ret));
        ret->classes = classes;
        ret->def     = def;
        ret->state   = def->start_state;
    }

    return ret;
}

void parser_reset(struct parser * parser) {
    parser->state   = parser->def->start_state;
}

const struct parser_event * parser_feed(struct parser * parser, const uint8_t c) {
    unsigned i;
    const unsigned type = parser->classes[c];

    parser->e1.next = parser->e2.next = 0;

    const struct parser_state_transition * state = parser->def->states[parser->state];
    const size_t n                               = parser->def->states_n[parser->state];
    bool matched;

    for (i = 0; i < n ; i++) {
        const int when = state[i].when;

        if (state[i].when <= 0xFF) {
            matched = (c == when);
        } else if (state[i].when == ANY) {
            matched = true;
        } else if (state[i].when > 0xFF) {
            matched = (bool) (type & when);
        } else {
            matched = false;
        }

        if (matched) {
            state[i].act1(&parser->e1, c);

            if (state[i].act2 != NULL) {
                parser->e1.next = &parser->e2;
                state[i].act2(&parser->e2, c);
            }

            parser->state = state[i].dest;
            break;
        }
    }

    return &parser->e1;
}

static const unsigned classes[0xFF] = {0x00};

const unsigned * parser_no_classes(void) {
    return classes;
}

