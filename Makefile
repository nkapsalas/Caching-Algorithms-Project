# Compiler settings
CC = gcc
CFLAGS = -g -Wall -Wextra -O3 -ffast-math -march=native -flto
LDFLAGS = -lm -lpthread -flto

TARGET_SIM = cache_simulator
TARGET_WSS = get_wss

# Source files and objects for the main simulator
SRCS_SIM = main.c LRU.c LFU-k.c LFU.c FF.c LRFU.c LIRS.c _2Q.c ARC.c
OBJS_SIM = $(SRCS_SIM:.c=.o)

HEADERS = cache.h uthash.h utlist.h

# Default rule
all: $(TARGET_SIM) $(TARGET_WSS)

# Build the main Cache Simulator
$(TARGET_SIM): $(OBJS_SIM)
	@echo "Linking object files to build $(TARGET_SIM)..."
	$(CC) $(CFLAGS) -o $(TARGET_SIM) $(OBJS_SIM) $(LDFLAGS)

# Build the WSS Helper program
$(TARGET_WSS): get_wss.c $(HEADERS)
	@echo "Compiling and linking $(TARGET_WSS)..."
	$(CC) $(CFLAGS) -o $(TARGET_WSS) get_wss.c

# General rule to compile .c files
%.o: %.c $(HEADERS)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	@echo "Cleaning up all build files..."
	rm -f $(OBJS_SIM) $(TARGET_SIM).exe $(TARGET_WSS).exe

.PHONY: all clean