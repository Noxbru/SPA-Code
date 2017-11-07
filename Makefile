CC = gcc
EXECUTABLE = spa

CFLAGS = -Wall -Wextra
OPTIMIZATION_FLAGS = -O2 -march=native
LDFLAGS = -lm

CFLAGS+=$(OPTIMIZATION_FLAGS)

SOURCES = Program.c ranvec.c
BIN = spa


all: spa

spa: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(BIN) $(LDFLAGS) 

clean: 
	@rm -f spa

