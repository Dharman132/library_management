CC=gcc
CFLAGS=-Wall -Wextra -O2

library: library.c
	$(CC) $(CFLAGS) -o library library.c

run: library
	./library

demo: library
	./library < demo_input.txt

clean:
	rm -f library

.PHONY: run demo clean
