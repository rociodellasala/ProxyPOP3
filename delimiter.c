#include <stdint.h>
#include <wchar.h>
#include <memory.h>
#include <stdlib.h>
#include "delimiter.h"
#include "parser_creator.h"
#include "parser_utils.h"
#include "mime_chars.h"

struct delimiter_st * delimiter_init() {
    
    struct delimiter_st *dlm = malloc(sizeof(*dlm));
    memset(dlm, 0, sizeof(*dlm));

    if (dlm != NULL) {
        dlm->delimiter_size = 2;
        dlm->delimiter[0] = '-';
        dlm->delimiter[1] = '-';
    }

    return dlm;
}

void close_delimiter(struct delimiter_st *delimiter) {
    
    if (delimiter != NULL) {
        delimiter->delimiter[delimiter->delimiter_size] = 0;

        if (delimiter->delimiter_parser != NULL) {
            parser_destroy(delimiter->delimiter_parser);
            parser_utils_strcmpi_destroy(delimiter->delimiter_parser_def);
            free(delimiter->delimiter_parser_def);
        }

        struct parser_definition *def = malloc(sizeof(*def));
        struct parser_definition aux = parser_utils_strcmpi(delimiter->delimiter);
        memcpy(def, &aux, sizeof(aux));
        def->states = aux.states;
        def->states_n = aux.states_n;
        delimiter->delimiter_parser = parser_init(init_char_class(), def);
        delimiter->delimiter_parser_def = def;

        delimiter->delimiter[delimiter->delimiter_size] = '-';
        delimiter->delimiter[delimiter->delimiter_size + 1] = '-';
        delimiter->delimiter[delimiter->delimiter_size + 2] = 0;

        if (delimiter->delimiter_end_parser != NULL) {
            parser_destroy(delimiter->delimiter_end_parser);
            parser_utils_strcmpi_destroy(delimiter->delimiter_end_parser_def);
            free(delimiter->delimiter_end_parser_def);
        }

        struct parser_definition *def_end = malloc(sizeof(*def_end));
        struct parser_definition aux_end = parser_utils_strcmpi(delimiter->delimiter);
        memcpy(def_end, &aux_end, sizeof(aux_end));
        def_end->states = aux_end.states;
        def_end->states_n = aux_end.states_n;
        delimiter->delimiter_end_parser = parser_init(init_char_class(), def_end);
        delimiter->delimiter_end_parser_def = def_end;
    }
}


void extend(const uint8_t c, struct delimiter_st *delimiter) {
    delimiter->delimiter[delimiter->delimiter_size] = c;
    delimiter->delimiter_size++;
}

void delimiter_reset(struct delimiter_st *delimiter) {
    parser_reset(delimiter->delimiter_end_parser);
    parser_reset(delimiter->delimiter_parser);
}

void delimiter_destroy(struct delimiter_st * delimiter){

    if (delimiter->delimiter_parser != NULL){
        parser_destroy(delimiter->delimiter_parser);
    }

    if (delimiter->delimiter_end_parser != NULL){
        parser_destroy(delimiter->delimiter_end_parser);
    }

    if (delimiter->delimiter_parser_def != NULL) {
        parser_utils_strcmpi_destroy(delimiter->delimiter_parser_def);
        free(delimiter->delimiter_parser_def);
    }

    if (delimiter->delimiter_end_parser_def != NULL) {
        parser_utils_strcmpi_destroy(delimiter->delimiter_end_parser_def);
        free(delimiter->delimiter_end_parser_def);
    }

    free(delimiter);
}
