#include "../include/utils.h"

uint16_t read_be16(uint8_t *ptr) {
    return (ptr[0] << 8) | ptr[1];
}

uint32_t read_be32(uint8_t *ptr) {
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}
