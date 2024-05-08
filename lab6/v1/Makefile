CC = gcc
# Show all warnings and treat them as errors
CFLAGS = -Wall -Werror

# List of source files
SRCS = pings.c pingc.c ip.c approutes.c approutec.c

# List of object files (will be generated from source files)
OBJS = $(SRCS:.c=.o)

# List of binary targets
TARGETS = pings.bin pingc.bin approutes.bin approutec.bin

# Headers for the binary
DEPS = ip.h constants.h

all: $(TARGETS)

pings.bin: pings.o ip.o
	$(CC) $(CFLAGS) $^ -o $@

pingc.bin: pingc.o ip.o
	$(CC) $(CFLAGS) $^ -o $@

approutes.bin: approutes.o ip.o
	$(CC) $(CFLAGS) $^ -o $@

approutec.bin: approutec.o ip.o
	$(CC) $(CFLAGS) $^ -o $@

# Compile each source file into object files
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGETS)
