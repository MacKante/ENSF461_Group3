CC=gcc
EXECUTABLES=gen_numbers.out gen_in_range.out select_arg.out
.DEFAULT_GOAL := all

OS_NAME := $(shell uname -s)
ifneq ($(OS_NAME), Linux)
all: @echo
else
all: $(EXECUTABLES)
endif

gen_numbers.out: gen_numbers.c utils.c
	$(CC) -o $@ $^

select_arg.out: select_arg.c utils.c
	$(CC) -o $@ $^

gen_in_range.out: gen_in_range.c utils.c
	$(CC) -o $@ $^

@echo:
	echo "Sorry, I prefer Linux"

clean:
	rm -f $(EXECUTABLES)

test:


.PHONY: all clean test