CC = gcc
# Show all warnings and treat them as errors
CFLAGS = -Wall -Werror

# List of source files
SRCS = pings.c ip.c

# List of object files (will be generated from source files)
OBJS = $(SRCS:.c=.o)

# Name of the output binary
TARGET = pings.bin

# Headers for the binary
DEPS = ip.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Compile each source file into object files
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
