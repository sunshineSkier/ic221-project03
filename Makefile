CC = gcc
CFLAGS = -g -Wall -lpthread -pthread
SRC = $(wildcard *.c)
BIN = $(SRC:.c=)

all: $(BIN)

submit: .submitted

.submitted: nvycat.c
	~/bin/submit -c=ic221 -p=proj3 $^
	@touch $@

clean:
	rm -f $(BIN) .submitted

.PHONY: all clean submit
