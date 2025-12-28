#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
  uint8_t magic[16];
  uint16_t page_size;
  uint8_t file_format_write;
  uint8_t file_format_read;
  uint8_t reserved_space;
  uint8_t max_embed_payload_frac;
  uint8_t min_embed_payload_frac;
  uint8_t leaf_payload_frac;
  uint32_t file_change_counter;
  uint32_t header_db_size;
  uint32_t first_freelist_trunk;
  uint32_t total_freelist_trunk;
  uint32_t schema_cookie;
  uint32_t schema_format_number;
  uint32_t default_page_cache_size;
  uint32_t page_number_largest_root;
  uint32_t db_text_encoding;
  uint32_t user_version;
  uint32_t incremental_version_mode;
  uint32_t application_id;
  uint8_t reserved_expansion[20];
  uint32_t version_valid_for;
  uint32_t sqlite_version_number;
} db_header_t;

// b-tree header section
typedef struct {
  uint8_t page_type;
  uint16_t first_freeblock;           // if zero, no free blocks
  uint16_t cell_count;
  uint16_t cell_content_start;        // if zero, value interpreted as 65536
  uint8_t fragmented_free_bytes;
  uint32_t rightmost_pointer;         // appears in the header of interior b-tree pages 
                                      // only and is omitted from all other pages.
  uint16_t *cell_pointers;
} btree_page_header_t;

// complete database structure
typedef struct {
    db_header_t header;
    btree_page_header_t *page_headers;
    void *file_data;
    size_t file_size;
} database_t;

#endif
