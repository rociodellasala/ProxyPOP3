#ifndef DELIMITER_H
#define DELIMITER_H

#include <stdint.h>

#define MAX_LENGTH 100


struct delimiter_st {
    char delimiter[MAX_LENGTH];
    uint8_t delimiter_size;
    struct parser_definition *delimiter_parser_def;
    struct parser_definition *delimiter_end_parser_def;
    struct parser *delimiter_parser;
    struct parser *delimiter_end_parser;
    
};


struct delimiter_st * delimiter_init();

void close_delimiter(struct delimiter_st *delimiter);

void delimiter_reset(struct delimiter_st *delimiter);

void delimiter_destroy(struct delimiter_st *delimiter);

void extend(const uint8_t c, struct delimiter_st *delimiter);



#endif 
