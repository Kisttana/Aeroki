# --- Configuration Variables ---

# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -D_GNU_SOURCE
LDFLAGS = -lm # -lm is often needed for math functions

# Target Executable Name
TARGET = aeroki

# Directories
BIN_DIR = bin
BUILD_DIR = build

# Include Paths (I-flags)
# Paths where header files are located, relative to the makefile
INCLUDE_DIRS = \
	-I Aeroki \
	-I Lexer \
	-I Parser \
	-I libs/tools \
	-I sysinfo \
	-I libs/Aegis/include

# List of all source files
SOURCES = \
	Aeroki/Aeroki.c \
	Lexer/Lexer.c \
	Parser/Parser.c \
	libs/tools/handlefile.c \
	libs/tools/print_error.c \
	libs/Aegis/src/aegis_utils.c \
	libs/Aegis/src/aegis_vector.c

# Automatically generate object file names, placing them in the build directory
# build/Aeroki/Aeroki.o, build/Lexer/Lexer.o, etc.
OBJECTS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SOURCES))

# Extract the unique directory paths needed under the build directory
# e.g., build/Aeroki, build/libs/Aegis/src
DIRS := $(sort $(dir $(OBJECTS)))


# --- Main Targets ---

.PHONY: all $(TARGET) clean dirs install

# Default target: build the executable
all: dirs $(BIN_DIR)/$(TARGET)

# Rule to link the object files into the final executable
$(BIN_DIR)/$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(LDFLAGS) $^ -o $@

# Rule to create all necessary build and bin directories
dirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(DIRS)


# --- Compilation Rule (Generic Pattern Rule) ---

# This rule handles compiling any .c file into its corresponding .o file
# and placing it inside the BUILD_DIR structure (e.g., build/Aeroki/Aeroki.o)
# The $< variable is the source file (e.g., Aeroki/Aeroki.c)
# The $@ variable is the target object file (e.g., build/Aeroki/Aeroki.o)
$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@


# --- Utility Targets ---

# Remove generated files
clean:
	@echo "Cleaning up build and binary directories..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)

# Install target (assuming you want to copy the executable to /usr/local/bin or similar)
install: all
	@echo "Installing $(TARGET) to $(BIN_DIR)..."
	@cp $(BIN_DIR)/$(TARGET) /usr/local/bin/$(TARGET) # Change target path as needed

# Placeholder for running the program
run: all
	@echo "Running $(TARGET)..."
	@./$(BIN_DIR)/$(TARGET) main.ark
