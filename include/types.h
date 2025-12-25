#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
  uint8_t magic[16]; // The header string: "SQLite format 3\000"
} db_header_t;

#endif
