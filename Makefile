# Target binary name
TARGET = Crox

# Directories
SRC_DIR = src
INCLUDE_DIR = include

# Source files (including mpc.c explicitly)
SRC_FILES = \
    $(SRC_DIR)/repl.c \
    $(SRC_DIR)/parser.c \
    $(SRC_DIR)/eval.c \
    $(SRC_DIR)/io.c \
    $(SRC_DIR)/ari.c \
    lib/mpc.c

# Compiler and flags
CC = gcc
CFLAGS = -I$(INCLUDE_DIR) -Wall -Wno-abi -std=c99
LDFLAGS = -lm -lreadline

# Build target
$(TARGET): $(SRC_FILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Run the built binary
run: $(TARGET)
	./$(TARGET)

# Clean target
clean:
	rm -f $(TARGET)
