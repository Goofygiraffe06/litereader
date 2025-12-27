#ifndef PARSER_H
#define PARSER_H

#include "types.h"

database_t* parse_database(const char *filename);
void free_database(database_t  *db);

#endif
