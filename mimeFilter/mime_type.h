#ifndef MIME_TYPE_H_
#define MIME_TYPE_H_

/**
 * mime_type.c 
 */
#include "parser_creator.h"

/** eventos en un mime type*/

enum mime_type_event_type {
    
    MIME_TYPE_TYPE,
    MIME_TYPE_TYPE_END,
    MIME_TYPE_SUBTYPE,
    MIME_PARAMETER_START,
    MIME_PARAMETER,
    MIME_BOUNDARY_END,
    MIME_DELIMITER_START,
    MIME_DELIMITER,
    MIME_DELIMITER_END,
    MIME_TYPE_UNEXPECTED,
};

const struct parser_definition * mime_type_parser(void);

const char *mime_type_event(enum mime_type_event_type type);


#endif