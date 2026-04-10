CC = gcc
CFLAGS = -Wall -Wextra -O2 -I.
LDFLAGS = -lX11 -lm

# Целевые файлы
all: run_noshadow run_shadow

run_noshadow: run_noshadow.c common/tk_drawer.h common/r3.h noshadow/polyedr.h
	$(CC) $(CFLAGS) -o run_noshadow run_noshadow.c $(LDFLAGS)

run_shadow: run_shadow.c common/tk_drawer.h common/r3.h shadow/polyedr.h
	$(CC) $(CFLAGS) -o run_shadow run_shadow.c $(LDFLAGS)

clean:
	rm -f run_noshadow run_shadow

.PHONY: all clean
