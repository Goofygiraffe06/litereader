#include "../include/utils.h"

uint16_t read_be16(uint8_t *ptr) {
    return (ptr[0] << 8) | ptr[1];
}

uint32_t read_be32(uint8_t *ptr) {
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

uint64_t read_varint(uint8_t *data, size_t *bytes_read) {
    uint64_t result = 0;
    for (int i = 0; i < 9; i++) {
        result = (result << 7) | (data[i] & 0x7F);
        if ((data[i] & 0x80) == 0) {
            *bytes_read = i + 1;
            return result;
        }
    }
    *bytes_read = 9;
    return result;
}
