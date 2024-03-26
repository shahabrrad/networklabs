CC = gcc
CFLAGS = -Wall -I.

all: remotecmdc.bin remotecmds.bin

remotecmdc.bin: remotecmdc.c parser.c parser.h ip.c ip.h
	$(CC) $(CFLAGS) -o remotecmdc.bin remotecmdc.c parser.c

remotecmds.bin: remotecmds.c parser.c parser.h ip.c ip.h
	$(CC) $(CFLAGS) -o remotecmds.bin remotecmds.c parser.c ip.c

clean:
	rm -f remotecmdc.bin remotecmds.bin

.PHONY: all clean
