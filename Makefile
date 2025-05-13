CC=c89
CFLAGS=-g -Wall -Wextra -Werror -pedantic

main: main.c svm.h
	@$(CC) $(CFLAGS) -o $@ $<
	@echo "CC  $@  <-  $^"
