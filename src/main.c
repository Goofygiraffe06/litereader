#include <stdio.h>
#include <string.h>
#include "../include/parser.h"
#include "../include/cell.h"
#include "../include/schema.h"
#include "../include/constants.h"

void print_db_header(db_header_t *header) {
    printf("magic: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x ", header->magic[i]);
    }
    printf("\n");
    printf("page size: %u\n", header->page_size);
    printf("file format write: %u\n", header->file_format_write);
    printf("file format read: %u\n", header->file_format_read);
    printf("reserved space: %u\n", header->reserved_space);
    printf("max embed payload frac: %u\n", header->max_embed_payload_frac);
    printf("min embed payload frac: %u\n", header->min_embed_payload_frac);
    printf("leaf payload frac: %u\n", header->leaf_payload_frac);
    printf("file change counter: %u\n", header->file_change_counter);
    printf("database size: %u pages\n", header->header_db_size);
    printf("first freelist trunk: %u\n", header->first_freelist_trunk);
    printf("total freelist pages: %u\n", header->total_freelist_trunk);
    printf("schema cookie: %u\n", header->schema_cookie);
    printf("schema format number: %u\n", header->schema_format_number);
    printf("default page cache size: %u\n", header->default_page_cache_size);
    printf("page number largest root: %u\n", header->page_number_largest_root);
    printf("text encoding: %u\n", header->db_text_encoding);
    printf("user version: %u\n", header->user_version);
    printf("incremental vacuum mode: %u\n", header->incremental_version_mode);
    printf("application id: %u\n", header->application_id);
    printf("reserved expansion: ");
    for (int i = 0; i < 20; i++) {
        printf("%02x ", header->reserved_expansion[i]);
    }
    printf("\n");
    printf("version valid for: %u\n", header->version_valid_for);
    printf("sqlite version number: %u\n", header->sqlite_version_number);
}

void print_page_header(btree_page_header_t *page, int page_num) {
    printf("\n=== Page %d Header ===\n", page_num);
    printf("page type: 0x%02x\n", page->page_type);
    printf("first freeblock: %u\n", page->first_freeblock);
    printf("cell count: %u\n", page->cell_count);
    printf("cell content start: %u\n", page->cell_content_start);
    printf("fragmented free bytes: %u\n", page->fragmented_free_bytes);
    
    if (page->page_type == PAGE_TYPE_INTERIOR_INDEX ||
        page->page_type == PAGE_TYPE_INTERIOR_TABLE) {
        printf("rightmost pointer: %u\n", page->rightmost_pointer);
    }
    
    if (page->cell_count > 0 && page->cell_pointers) {
        printf("cell pointers: ");
        for (uint16_t i = 0; i < page->cell_count; i++) {
            printf("%u ", page->cell_pointers[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s <file.db>\n", argv[0]);
        return 1;
    }
    
    database_t *db = parse_database(argv[1]);
    if (!db) {
        printf("failed to parse database\n");
        return 1;
    }
    
    if (memcmp(db->header.magic, SQLITE_MAGIC, 16) != 0) {
        printf("invalid sqlite file\n");
        free_database(db);
        return 1;
    }
    
    print_db_header(&db->header);
    
    // parse and print schema
    schema_t *schema = parse_schema(db);
    if (schema) {
        print_schema(schema);
        free_schema(schema);
    }
    
    // parse and print all pages
    for (uint32_t i = 0; i < db->header.header_db_size; i++) {
        print_page_header(&db->page_headers[i], i + 1);
        
        // parse cells for leaf table pages
        if (db->page_headers[i].page_type == PAGE_TYPE_LEAF_TABLE) {
            printf("\nCells:\n");
            
            size_t page_offset = (i == 0) ? 0x64 : (db->header.page_size * i);
            uint8_t *page_data = (uint8_t *)db->file_data + page_offset;
            
            for (uint16_t j = 0; j < db->page_headers[i].cell_count; j++) {
                parse_cell(page_data, db->page_headers[i].cell_pointers[j], db->header.page_size);
            }
        }
    }
    
    free_database(db);
    return 0;
}
