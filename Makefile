# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

liteparser: src/parser.c
	$(CC) $(CFLAGS) -o bin/litereader src/main.c src/parser.c src/utils.c

clean:
	rm -f bin/litereader

test: liteparser
	bin/litereader tests/db/test.db
