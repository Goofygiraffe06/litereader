#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>

uint16_t read_be16(uint8_t *ptr);
uint32_t read_be32(uint8_t *ptr);
uint64_t read_varint(uint8_t *data, size_t *bytes_read, size_t max_len);

#endif
