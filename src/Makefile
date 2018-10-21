CC=gcc
CFLAGS=-Wall -Wextra -Werror -g

SRCDIR = ./src
OBJDIR = ./build
BINDIR = ./bin
TARGET = main

SOURCES = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/**/*.c) $(wildcard $(SRCDIR)/**/**/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

dir_guard=@mkdir -p $(@D)

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(dir_guard)
	$(CC) -I. -I./include $+ -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(dir_guard)
	$(CC) $(CFLAGS) -c $< -I./include -o $@

run: $(BINDIR)/$(TARGET)
	$(BINDIR)/$(TARGET) ${ARGS}

clean:
	rm -rvf $(BINDIR) $(OBJDIR)

