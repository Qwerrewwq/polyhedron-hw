# Makefile for Polyedr C Translation

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -lm -lX11

# Source files
SRCS = polyedr.c tk_drawer.c
OBJS = $(SRCS:.c=.o)

# Targets
all: test_polyedr run_shadow run_noshadow

# Test executable (no GUI required)
test_polyedr: test_polyedr.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Shadow version with GUI
run_shadow: run_shadow.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# No-shadow version with GUI
run_noshadow: run_noshadow.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean
clean:
	rm -f *.o test_polyedr run_shadow run_noshadow

# Run tests
test: test_polyedr
	./test_polyedr

.PHONY: all clean test
