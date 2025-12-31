#ifndef SCHEMA_H
#define SCHEMA_H

#include "types.h"

schema_t* parse_schema(database_t *db);
void free_schema(schema_t *schema);
void print_schema(schema_t *schema);

#endif
