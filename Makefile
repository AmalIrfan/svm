CC=c89
CFLAGS=-g -Wall -Wextra -Werror -pedantic
CFLAGS+=-I.

all: build build/main

build:
	mkdir -p build

build/%: %.c svm.h
	@$(CC) $(CFLAGS) -o $@ $<
	@echo "CC  $@  <-  $^"

EXAMPLES=$(wildcard examples/*.c)

examples: build/examples $(patsubst %.c,build/%,$(EXAMPLES))

build/examples:
	mkdir -p build/examples
