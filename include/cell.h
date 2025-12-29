#ifndef CELL_H
#define CELL_H

#include <stdint.h>
#include <stddef.h>

int parse_cell(uint8_t *page_data, uint16_t cell_offset, size_t page_size);

#endif
