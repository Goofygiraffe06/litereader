#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "../include/serializer.h"

void json_print_text_chk(const uint8_t *data, size_t len) {
    if (!data) {
        printf("null");
        return;
    }
    
    printf("\"");
    for (size_t i = 0; i < len; i++) {
        uint8_t c = data[i];
        if (c == '"') printf("\\\"");
        else if (c == '\\') printf("\\\\");
        else if (c == '\b') printf("\\b");
        else if (c == '\f') printf("\\f");
        else if (c == '\n') printf("\\n");
        else if (c == '\r') printf("\\r");
        else if (c == '\t') printf("\\t");
        else if (c < 32) printf("\\u%04x", c);
        else printf("%c", c);
    }
    printf("\"");
}

void json_print_string(const char *str) {
    if (!str) {
        printf("null");
        return;
    }
    json_print_text_chk((const uint8_t*)str, strlen(str));
}

void serialize_db_header(db_header_t *header) {
    printf("\"header\": {\n");
    printf("    \"page_size\": %u,\n", header->page_size);
    printf("    \"file_format_write\": %u,\n", header->file_format_write);
    printf("    \"file_format_read\": %u,\n", header->file_format_read);
    printf("    \"reserved_space\": %u,\n", header->reserved_space);
    printf("    \"max_embed_payload_frac\": %u,\n", header->max_embed_payload_frac);
    printf("    \"min_embed_payload_frac\": %u,\n", header->min_embed_payload_frac);
    printf("    \"leaf_payload_frac\": %u,\n", header->leaf_payload_frac);
    printf("    \"file_change_counter\": %u,\n", header->file_change_counter);
    printf("    \"header_db_size\": %u,\n", header->header_db_size);
    printf("    \"first_freelist_trunk\": %u,\n", header->first_freelist_trunk);
    printf("    \"total_freelist_pages\": %u,\n", header->total_freelist_trunk);
    printf("    \"schema_cookie\": %u,\n", header->schema_cookie);
    printf("    \"schema_format_number\": %u,\n", header->schema_format_number);
    printf("    \"default_page_cache_size\": %u,\n", header->default_page_cache_size);
    printf("    \"page_number_largest_root\": %u,\n", header->page_number_largest_root);
    printf("    \"db_text_encoding\": %u,\n", header->db_text_encoding);
    printf("    \"user_version\": %u,\n", header->user_version);
    printf("    \"incremental_vacuum_mode\": %u,\n", header->incremental_version_mode);
    printf("    \"application_id\": %u,\n", header->application_id);
    printf("    \"version_valid_for\": %u,\n", header->version_valid_for);
    printf("    \"sqlite_version_number\": %u\n", header->sqlite_version_number);
    printf("  }"); // End header
}

void serialize_schema(schema_t *schema) {
    printf("\"schema\": [");
    if (schema) {
        for (size_t i = 0; i < schema->count; i++) {
            if (i > 0) printf(",");
            schema_entry_t *e = &schema->entries[i];
            printf("\n    {");
            printf("\"type\": "); json_print_string(e->type); printf(",");
            printf("\"name\": "); json_print_string(e->name); printf(",");
            printf("\"tbl_name\": "); json_print_string(e->tbl_name); printf(",");
            printf("\"rootpage\": %llu,", (unsigned long long)e->rootpage);
            printf("\"sql\": "); json_print_string(e->sql);
            printf("}");
        }
    }
    printf("\n  ]"); // End schema
}

void serialize_page_header(btree_page_header_t *page, int page_num) {
    printf("    {\n");
    printf("      \"page_num\": %d,\n", page_num);
    printf("      \"header\": {\n");
    printf("        \"page_type\": %u,\n", page->page_type);
    printf("        \"first_freeblock\": %u,\n", page->first_freeblock);
    printf("        \"cell_count\": %u,\n", page->cell_count);
    printf("        \"cell_content_start\": %u,\n", page->cell_content_start);
    printf("        \"fragmented_free_bytes\": %u", page->fragmented_free_bytes);
    if (page->page_type == 0x02 || page->page_type == 0x05) {
        printf(",\n        \"rightmost_pointer\": %u", page->rightmost_pointer);
    }
    printf("\n      },\n");
    printf("      \"cells\": [");
    // Cells array will be populated by caller
}
