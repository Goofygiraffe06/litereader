#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cell.h"
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

int parse_cell(uint8_t *page_data, uint16_t cell_offset, size_t page_size) {
    if (cell_offset >= page_size) {
        return -1;
    }
    
    uint8_t *cell = page_data + cell_offset;
    size_t offset = 0;
    size_t bytes_read;
    size_t remaining = page_size - cell_offset;
    
    if (remaining < 1) return -1;
    read_varint(cell + offset, &bytes_read, remaining - offset);
    if (bytes_read == 0) {
        return -1;
    }
    offset += bytes_read;
    
    if (remaining < offset + 1) return -1;
    uint64_t rowid = read_varint(cell + offset, &bytes_read, remaining - offset);
    if (bytes_read == 0) {
        return -1;
    }
    offset += bytes_read;
    
    if (remaining < offset + 1) return -1;
    size_t header_start = offset;
    uint64_t header_size = read_varint(cell + offset, &bytes_read, remaining - offset);
    if (bytes_read == 0 || header_size > remaining - offset) {
        return -1;
    }
    offset += bytes_read;
    
    // Use dynamic allocation with reallocation as needed
    size_t capacity = 16;  // Start small
    uint64_t *serial_types = malloc(sizeof(uint64_t) * capacity);
    if (!serial_types) {
        return -1;
    }
    
    size_t col_count = 0;
    
    while (offset < header_start + header_size && offset < remaining) {
        // Reallocate if needed
        if (col_count >= capacity) {
            size_t new_capacity = capacity * 2;
            uint64_t *new_serial_types = realloc(serial_types, sizeof(uint64_t) * new_capacity);
            if (!new_serial_types) {
                free(serial_types);
                return -1;
            }
            serial_types = new_serial_types;
            capacity = new_capacity;
        }
        
        serial_types[col_count] = read_varint(cell + offset, &bytes_read, remaining - offset);
        if (bytes_read == 0) break;
        offset += bytes_read;
        col_count++;
    }
    
    // print rowid
    printf("rowid: %lu | ", rowid);
    
    // read and print values
    for (size_t i = 0; i < col_count; i++) {
        uint64_t serial_type = serial_types[i];
        size_t content_size = get_serial_content_size(serial_type);
        
        if (i > 0) printf(", ");
        
        if (offset + content_size > remaining) {
            printf("(truncated)");
            break;
        }
        
        if (serial_type == SERIAL_TYPE_NULL) {
            printf("NULL");
        } else if (serial_type == SERIAL_TYPE_ZERO) {
            printf("0");
        } else if (serial_type == SERIAL_TYPE_ONE) {
            printf("1");
        } else if (serial_type >= SERIAL_TYPE_INT8 && serial_type <= SERIAL_TYPE_INT64) {
            int64_t value = read_int_value(cell + offset, content_size);
            printf("%ld", value);
            offset += content_size;
        } else if (serial_type >= 13 && serial_type % 2 == 1) {
            // text
            printf("\"");
            for (size_t j = 0; j < content_size; j++) {
                printf("%c", cell[offset + j]);
            }
            printf("\"");
            offset += content_size;
        } else if (serial_type >= 12 && serial_type % 2 == 0) {
            // blob
            printf("BLOB(%zu bytes)", content_size);
            offset += content_size;
        } else {
            printf("(unknown)");
            offset += content_size;
        }
    }
    
    free(serial_types);
    printf("\n");
    return 0;
}
