CFLAGS=-std=gnu99 -g -Wall -Wextra -Werror -pedantic
CFLAGS+=-I.

all: build/main build/sas

build/%: %.c svm.h
	@mkdir -p build
	@$(CC) $(CFLAGS) -o $@ $<
	@echo "CC  $@  <-  $^"

EXAMPLES=$(wildcard examples/*.c)

examples: build/examples $(patsubst %.c,build/%,$(EXAMPLES))

build/examples:
	mkdir -p build/examples
