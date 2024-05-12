# Compiler and Compiler Flags
CC=gcc
CFLAGS=-g -Iinclude -Idrivers
# Linker flags
LDFLAGS=-lreadline -lrt -lpthread

# The build target executable:
TARGET=minios

# Source, Object files
SRCS= boot/boot.c \
	drivers/helper.c \
	drivers/primitive.c \
	drivers/proc.c \
	kernel/core.c \
	kernel/input.c \
	kernel/stat.c \
	kernel/ui.c \
	kernel/20150498/pi.c

OBJS=$(SRCS:.c=.o) 

# Include directory
INCLUDE_DIR=include

all: $(TARGET) install_dependency

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# To obtain object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up:`
clean:
	rm -f $(OBJS) $(TARGET)

install_dependency:
	sudo ./prepare.sh