CC=g++
CFLAGS=-Wall -Wextra -Werror -g
LDFLAGS=-lncurses

SRCDIR = ./src
OBJDIR = ./build
BINDIR = ./bin
TARGET = main

SOURCES = $(wildcard $(SRCDIR)/*.cpp $(SRCDIR)/**/*.cpp $(SRCDIR)/**/**/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

dir_guard=@mkdir -p $(@D)

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(dir_guard)
	$(CC) -I. -I./include $+ -o $@ $(LDFLAGS)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(dir_guard)
	$(CC) $(CFLAGS) -c $< -I./include -o $@

run: $(BINDIR)/$(TARGET)
	$(BINDIR)/$(TARGET) ${ARGS}

clean:
	rm -rvf $(BINDIR) $(OBJDIR)

