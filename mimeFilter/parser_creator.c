#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "parser_creator.h"
#include "mimeList.h"
#include "mimeFilter.h"

struct parser *parser_init(const unsigned *classes, const struct parser_definition *def) {
    struct parser *ret = malloc(sizeof(*ret));
    if(ret != NULL) {
        memset(ret, 0, sizeof(*ret));
        ret->classes = classes;
        ret->def     = def;
        ret->state   = def->start_state;
    }
    return ret;
}

void parser_destroy(struct parser *p) {
    if(p != NULL) {
        free(p);
    }
}




