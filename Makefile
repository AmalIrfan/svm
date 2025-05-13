CC=c89
CFLAGS=-g -Wall -Wextra -Werror -pedantic
CFLAGS+=-I.

all: main

%: %.c svm.h
	@$(CC) $(CFLAGS) -o $@ $<
	@echo "CC  $@  <-  $^"

EXAMPLES=$(wildcard examples/*.c)

examples: $(patsubst %.c,%,$(EXAMPLES))
