# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

liteparser: src/parser.c
	$(CC) $(CFLAGS) -o bin/liteparser src/main.c src/parser.c src/utils.c

clean:
	rm -f bin/liteparser

test: liteparser
	bin/liteparser /tests/db/test.db
