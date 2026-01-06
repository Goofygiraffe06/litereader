#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/schema.h"
#include "../include/utils.h"
#include "../include/constants.h"

static size_t get_serial_content_size(uint64_t serial_type) {
    if (serial_type >= 12) {
        if (serial_type % 2 == 0) {
            return (serial_type - 12) / 2;
        } else {
            return (serial_type - 13) / 2;
        }
    }
    
    switch (serial_type) {
        case SERIAL_TYPE_NULL: return 0;
        case SERIAL_TYPE_INT8: return 1;
        case SERIAL_TYPE_INT16: return 2;
        case SERIAL_TYPE_INT24: return 3;
        case SERIAL_TYPE_INT32: return 4;
        case SERIAL_TYPE_INT48: return 6;
        case SERIAL_TYPE_INT64: return 8;
        case SERIAL_TYPE_FLOAT64: return 8;
        case SERIAL_TYPE_ZERO: return 0;
        case SERIAL_TYPE_ONE: return 0;
        default: return 0;
    }
}

static int64_t read_int_value(uint8_t *data, size_t size) {
    int64_t value = 0;
    for (size_t i = 0; i < size; i++) {
        value = (value << 8) | data[i];
    }
    
    // sign extend for negative values
    if (size < 8 && (data[0] & 0x80)) {
        for (size_t i = size; i < 8; i++) {
            value |= ((int64_t)0xFF << (i * 8));
        }
    }
    
    return value;
}

static char* read_text_value(uint8_t *data, size_t size) {
    char *str = malloc(size + 1);
    if (!str) return NULL;
    memcpy(str, data, size);
    str[size] = '\0';
    return str;
}

static int parse_schema_cell(uint8_t *page_data, uint16_t cell_offset, 
                             size_t page_size, schema_entry_t *entry) {
    if (cell_offset >= page_size) {
        return -1;
    }
    
    uint8_t *cell = page_data + cell_offset;
    size_t offset = 0;
    size_t bytes_read;
    size_t remaining = page_size - cell_offset;
    
    if (remaining < 1) return -1;
    read_varint(cell + offset, &bytes_read, remaining - offset);
    if (bytes_read == 0) return -1;
    offset += bytes_read;
    
    if (remaining < offset + 1) return -1;
    read_varint(cell + offset, &bytes_read, remaining - offset);
    if (bytes_read == 0) return -1;
    offset += bytes_read;
    
    if (remaining < offset + 1) return -1;
    size_t header_start = offset;
    uint64_t header_size = read_varint(cell + offset, &bytes_read, remaining - offset);
    if (bytes_read == 0 || header_size > remaining - offset) return -1;
    offset += bytes_read;
    
    // schema has 5 columns: type, name, tbl_name, rootpage, sql
    uint64_t serial_types[5];
    size_t col_count = 0;
    
    while (offset < header_start + header_size && col_count < 5) {
        serial_types[col_count] = read_varint(cell + offset, &bytes_read, remaining - offset);
        if (bytes_read == 0) break;
        offset += bytes_read;
        col_count++;
    }
    
    if (col_count < 5) return -1;
    
    offset = header_start + header_size;
    
    size_t content_size = get_serial_content_size(serial_types[0]);
    if (serial_types[0] >= 13 && serial_types[0] % 2 == 1) {
        entry->type = read_text_value(cell + offset, content_size);
    } else {
        entry->type = NULL;
    }
    offset += content_size;
    
    // read name
    content_size = get_serial_content_size(serial_types[1]);
    if (serial_types[1] >= 13 && serial_types[1] % 2 == 1) {
        entry->name = read_text_value(cell + offset, content_size);
    } else {
        entry->name = NULL;
    }
    offset += content_size;
    
    // read tbl_name
    content_size = get_serial_content_size(serial_types[2]);
    if (serial_types[2] >= 13 && serial_types[2] % 2 == 1) {
        entry->tbl_name = read_text_value(cell + offset, content_size);
    } else {
        entry->tbl_name = NULL;
    }
    offset += content_size;
    
    // read rootpage
    content_size = get_serial_content_size(serial_types[3]);
    if (serial_types[3] >= SERIAL_TYPE_INT8 && serial_types[3] <= SERIAL_TYPE_INT64) {
        entry->rootpage = read_int_value(cell + offset, content_size);
    } else if (serial_types[3] == SERIAL_TYPE_ZERO) {
        entry->rootpage = 0;
    } else if (serial_types[3] == SERIAL_TYPE_ONE) {
        entry->rootpage = 1;
    } else {
        entry->rootpage = 0;
    }
    offset += content_size;
    
    // read sql 
    content_size = get_serial_content_size(serial_types[4]);
    if (serial_types[4] >= 13 && serial_types[4] % 2 == 1) {
        entry->sql = read_text_value(cell + offset, content_size);
    } else {
        entry->sql = NULL;
    }
    
    return 0;
}

schema_t* parse_schema(database_t *db) {
    if (!db || db->header.header_db_size == 0) {
        return NULL;
    }
    
    btree_page_header_t *page = &db->page_headers[0];
    
    if (page->page_type != PAGE_TYPE_LEAF_TABLE) {
        return NULL;
    }
    
    schema_t *schema = malloc(sizeof(schema_t));
    if (!schema) return NULL;
    
    schema->capacity = page->cell_count;
    schema->count = 0;
    schema->entries = malloc(sizeof(schema_entry_t) * schema->capacity);
    if (!schema->entries) {
        free(schema);
        return NULL;
    }
    
    uint8_t *page_data = (uint8_t *)db->file_data;
    
    for (uint16_t i = 0; i < page->cell_count; i++) {
        schema_entry_t entry = {0};
        if (parse_schema_cell(page_data, page->cell_pointers[i], 
                              db->header.page_size, &entry) == 0) {
            schema->entries[schema->count++] = entry;
        }
    }
    
    return schema;
}

void free_schema(schema_t *schema) {
    if (!schema) return;
    
    for (size_t i = 0; i < schema->count; i++) {
        free(schema->entries[i].type);
        free(schema->entries[i].name);
        free(schema->entries[i].tbl_name);
        free(schema->entries[i].sql);
    }
    free(schema->entries);
    free(schema);
}

void print_schema(schema_t *schema) {
    if (!schema) return;
    
    printf("\n=== Database Schema ===\n");
    for (size_t i = 0; i < schema->count; i++) {
        schema_entry_t *e = &schema->entries[i];
        printf("\n[%zu] %s: %s\n", i + 1, 
               e->type ? e->type : "(null)",
               e->name ? e->name : "(null)");
        printf("    table: %s\n", e->tbl_name ? e->tbl_name : "(null)");
        printf("    rootpage: %llu\n", (unsigned long long)e->rootpage);
        printf("    sql: %s\n", e->sql ? e->sql : "(null)");
    }
}
