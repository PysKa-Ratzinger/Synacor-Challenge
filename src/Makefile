CC=gcc
OBJECTS=main.o machine.o stack.o machine_debug.o circular_array.o
CFLAGS=-Wall -Wextra -Werror -g

all: main

%.o: %.c %.h

main: $(OBJECTS)
	gcc -o main $(CFLAGS) $^

run: main
	./main

clean:
	rm -rvf ./main $(OBJECTS)

