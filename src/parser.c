#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../include/parser.h"
#include "../include/constants.h"
#include "../include/utils.h"

database_t* parse_database(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return NULL;
    }

    database_t *db = malloc(sizeof(database_t));
    if (!db) {
        close(fd);
        return NULL;
    }

    // parse database header
    uint8_t header_buffer[0x64];
    if (read(fd, header_buffer, 0x64) != 0x64) {
        perror("read");
        close(fd);
        free(db);
        return NULL;
    }

    memcpy(db->header.magic, header_buffer + OFFSET_MAGIC, 16);
    db->header.page_size = read_be16(header_buffer + OFFSET_PAGE_SIZE);
    db->header.file_format_write = header_buffer[OFFSET_FILE_FORMAT_WRITE_VERSION];
    db->header.file_format_read = header_buffer[OFFSET_FILE_FORMAT_READ];
    db->header.reserved_space = header_buffer[OFFSET_RESERVED_SPACE];
    db->header.max_embed_payload_frac = header_buffer[OFFSET_MAX_EMBED_PAYLOAD_FRAC];
    db->header.min_embed_payload_frac = header_buffer[OFFSET_MIN_EMBED_PAYLOAD_FRAC];
    db->header.leaf_payload_frac = header_buffer[OFFSET_LEAF_PAYLOAD_FRAC];
    db->header.file_change_counter = read_be32(header_buffer + OFFSET_FILE_CHANGE_COUNTER);
    db->header.header_db_size = read_be32(header_buffer + OFFSET_HEADER_DB_SIZE);
    db->header.first_freelist_trunk = read_be32(header_buffer + OFFSET_FIRST_FREELIST_TRUNK);
    db->header.total_freelist_trunk = read_be32(header_buffer + OFFSET_TOTAL_FREELIST_PAGES);
    db->header.schema_cookie = read_be32(header_buffer + OFFSET_SCHEMA_COOKIE);
    db->header.schema_format_number = read_be32(header_buffer + OFFSET_SCHEMA_FORMAT_NUMBER);
    db->header.default_page_cache_size = read_be32(header_buffer + OFFSET_DEFAULT_PAGE_CACHE_SIZE);
    db->header.page_number_largest_root = read_be32(header_buffer + OFFSET_PAGE_NUMBER_LARGEST_ROOT);
    db->header.db_text_encoding = read_be32(header_buffer + OFFSET_DB_TEXT_ENCODING);
    db->header.user_version = read_be32(header_buffer + OFFSET_USER_VERSION);
    db->header.incremental_version_mode = read_be32(header_buffer + OFFSET_INCREMENTAL_VACCUM_MODE);
    db->header.application_id = read_be32(header_buffer + OFFSET_APPLICATION_ID);
    memcpy(db->header.reserved_expansion, header_buffer + OFFSET_RESERVED_EXPANSION, 20);
    db->header.version_valid_for = read_be32(header_buffer + OFFSET_VERSION_VALID_FOR);
    db->header.sqlite_version_number = read_be32(header_buffer + OFFSET_SQLITE_VERSION_NUMBER);

    // allocate page_headers array
    uint32_t page_count = db->header.header_db_size;
    db->page_headers = malloc(sizeof(btree_page_header_t) * page_count);
    if (!db->page_headers) {
        close(fd);
        free(db);
        return NULL;
    }

    // parse all page headers
    for (uint32_t i = 0; i < page_count; i++) {
        size_t page_offset = (i == 0) ? 0x64 : (db->header.page_size * i);

        lseek(fd, page_offset, SEEK_SET);

        uint8_t page_buffer[12];
        if (read(fd, page_buffer, 12) != 12) {
            perror("read");
            close(fd);
            free(db->page_headers);
            free(db);
            return NULL;
        }

        db->page_headers[i].page_type = page_buffer[OFFSET_BTREE_PAGE_TYPE];
        db->page_headers[i].first_freeblock = read_be16(page_buffer + OFFSET_BTREE_FIRST_FREEBLOCK);
        db->page_headers[i].cell_count = read_be16(page_buffer + OFFSET_BTREE_CELL_COUNT);
        db->page_headers[i].cell_content_start = read_be16(page_buffer + OFFSET_BTREE_CELL_CONTENT_START);
        db->page_headers[i].fragmented_free_bytes = page_buffer[OFFSET_BTREE_FRAG_FREE_BYTES];

        if (db->page_headers[i].page_type == PAGE_TYPE_INTERIOR_INDEX ||
            db->page_headers[i].page_type == PAGE_TYPE_INTERIOR_TABLE) {
            db->page_headers[i].rightmost_pointer = read_be32(page_buffer + OFFSET_BTREE_RIGHTMOST_POINTER);
        }
    }

    close(fd);
    return db;
}

void free_database(database_t *db) {
    if (db) {
        free(db->page_headers);
        free(db);
    }
}
