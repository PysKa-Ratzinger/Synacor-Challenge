CC=gcc
OBJECTS=main.o machine.o stack.o
CFLAGS=-Wall -Wextra -Werror -g

all: main

%.o: %.c %.h

main: $(OBJECTS)
	gcc -o main $(CFLAGS) $^

clean:
	rm -rvf ./main $(OBJECTS)

