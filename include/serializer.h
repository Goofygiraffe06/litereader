#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "../include/parser.h"
#include "../include/schema.h"

void json_print_string(const char *str);
void json_print_text_chk(const uint8_t *data, size_t len);
void serialize_db_header(db_header_t *header);
void serialize_schema(schema_t *schema);
void serialize_page_header(btree_page_header_t *page, int page_num);

#endif
