CC = gcc
# Show all warnings and treat them as errors
CFLAGS = -Wall -Werror -D_GNU_SOURCE

# List of source files
SRCS = ssftp.c ssftpd.c ip.c utils.c

# List of object files (will be generated from source files)
OBJS = $(SRCS:.c=.o)

# List of binary targets
TARGETS = ssftp.bin ssftpd.bin

# Headers for the binary
DEPS = ip.h utils.h constants.h

all: $(TARGETS)

ssftp.bin: ssftp.o ip.o
	$(CC) $(CFLAGS) $^ -o $@

ssftpd.bin: ssftpd.o ip.o utils.o
	$(CC) $(CFLAGS) $^ -o $@

# Compile each source file into object files
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGETS)
