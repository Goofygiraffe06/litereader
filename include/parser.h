#ifndef PARSER_H
#define PARSER_H

#include "types.h"

db_header_t* parse_header(const char *filename);
void free_header(db_header_t *header);

#endif
