#ifndef PARSER_CREATOR_H_
#define PARSER_CREATOR_H_


struct parser *parser_init    (const unsigned *classes, const struct parser_definition *def);

void parser_destroy(struct parser *p);

struct parser_definition create_parser_def_message();

struct parser_definition create_parser_def_multi_pop();

struct parser_definition create_mimeType_def_parser();

#endif