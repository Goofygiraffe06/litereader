# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

liteparser: src/parser.c
	$(CC) $(CFLAGS) -o bin/liteparser src/parser.c

clean:
	rm -f liteparser

test: liteparser
	./liteparser test.db
