// src/parser.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../include/parser.h"
#include "../include/constants.h"
#include "../include/utils.h"

db_header_t* parse_header(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return NULL;
    }
    
    uint8_t buffer[0x64];
    if (read(fd, buffer, 0x64) != 0x64) {
        perror("read");
        close(fd);
        return NULL;
    }
    
    close(fd);
    
    db_header_t *header = malloc(sizeof(db_header_t));
    if (!header) {
        return NULL;
    }
    
    memcpy(header->magic, buffer + OFFSET_MAGIC, 16);
    header->page_size = read_be16(buffer + OFFSET_PAGE_SIZE);
    header->file_format_write = buffer[OFFSET_FILE_FORMAT_WRITE_VERSION];
    header->file_format_read = buffer[OFFSET_FILE_FORMAT_READ];
    header->reserved_space = buffer[OFFSET_RESERVED_SPACE];
    header->max_embed_payload_frac = buffer[OFFSET_MAX_EMBED_PAYLOAD_FRAC];
    header->min_embed_payload_frac = buffer[OFFSET_MIN_EMBED_PAYLOAD_FRAC];
    header->leaf_payload_frac = buffer[OFFSET_LEAF_PAYLOAD_FRAC];
    header->file_change_counter = read_be32(buffer + OFFSET_FILE_CHANGE_COUNTER);
    header->header_db_size = read_be32(buffer + OFFSET_HEADER_DB_SIZE);
    header->first_freelist_trunk = read_be32(buffer + OFFSET_FIRST_FREELIST_TRUNK);
    header->total_freelist_trunk = read_be32(buffer + OFFSET_TOTAL_FREELIST_PAGES);
    header->schema_cookie = read_be32(buffer + OFFSET_SCHEMA_COOKIE);
    header->schema_format_number = read_be32(buffer + OFFSET_SCHEMA_FORMAT_NUMBER);
    header->default_page_cache_size = read_be32(buffer + OFFSET_DEFAULT_PAGE_CACHE_SIZE);
    header->page_number_largest_root = read_be32(buffer + OFFSET_PAGE_NUMBER_LARGEST_ROOT);
    header->db_text_encoding = read_be32(buffer + OFFSET_DB_TEXT_ENCODING);
    header->user_version = read_be32(buffer + OFFSET_USER_VERSION);
    header->incremental_version_mode = read_be32(buffer + OFFSET_INCREMENTAL_VACCUM_MODE);
    header->application_id = read_be32(buffer + OFFSET_APPLICATION_ID);
    memcpy(header->reserved_expansion, buffer + OFFSET_RESERVED_EXPANSION, 20);
    header->version_valid_for = read_be32(buffer + OFFSET_VERSION_VALID_FOR);
    header->sqlite_version_number = read_be32(buffer + OFFSET_SQLITE_VERSION_NUMBER);
    
    return header;
}

void free_header(db_header_t *header) {
    free(header);
}
