#include <stdio.h>
#include <string.h>
#include "../include/parser.h"
#include "../include/constants.h"

void print_header(db_header_t *header) {
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

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s <file.db>\n", argv[0]);
        return 1;
    }
    
    db_header_t *header = parse_header(argv[1]);
    if (!header) {
        printf("failed to parse header\n");
        return 1;
    }
    
    if (memcmp(header->magic, SQLITE_MAGIC, 16) != 0) {
        printf("invalid sqlite file\n");
        free_header(header);
        return 1;
    }
    
    print_header(header);
    
    free_header(header);
    return 0;
}
