CC=gcc
EXECUTABLES=gen_numbers.out gen_in_range.out select_arg.out
.DEFAULT_GOAL := all

OS_NAME := $(shell uname -s)
ifneq ($(OS_NAME), Linux)
all:
	@echo "Sorry, I prefer Linux"
else
all: $(EXECUTABLES)
endif

gen_numbers.out: gen_numbers.c utils.c
	$(CC) -o $@ $^

select_arg.out: select_arg.c utils.c
	$(CC) -o $@ $^

gen_in_range.out: gen_in_range.c utils.c
	$(CC) -o $@ $^

clean:
	rm -f $(EXECUTABLES)

test: $(EXECUTABLES)
	./select_arg.out 1 2 3 || exit_code=$$?; \
	echo "$$exit_code"; \
    if [ $$exit_code -ne 0 ]; then \
        echo "Yikes"; \
    fi; \
    exit 0

	./gen_numbers.out 3 || exit_code=$$?; \
	echo "$$exit_code"; \
    if [ $$exit_code -ne 0 ]; then \
        echo "Yikes"; \
    fi; \
    exit 0

	./gen_in_range.out 1 10 || exit_code=$$?; \
	echo "$$exit_code"; \
    if [ $$exit_code -ne 0 ]; then \
        echo "Yikes"; \
    fi; \
    exit 0
.PHONY: all clean test
