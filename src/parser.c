#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../include/types.h"
#include "../include/constants.h"

db_header_t* load_header(const char *filename) {
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
    
    return header;
}

void free_header(db_header_t *header) {
    free(header);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: file.db");
        return 1;
    }
    
    db_header_t *header = load_header(argv[1]);
    if (!header) {
        printf("failed to load header\n");
        return 1;
    }
    
    if (memcmp(header->magic, SQLITE_MAGIC, 16) != 0) {
        printf("invalid sqlite file\n");
        return 1;
    }

    printf("magic: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x ", header->magic[i]);
    }
    printf("\n");

    free_header(header);
    return 0;
}
