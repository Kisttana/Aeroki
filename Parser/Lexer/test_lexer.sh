#!/bin/bash

# ==========================================
# Configuration
# ==========================================
APP_NAME="ark_lexer"
CC="gcc"
# Flags: -g for debug symbols, -Wall for warnings, -I. to include current dir
CFLAGS="-g -Wall -Wextra -I."
LD="../../libs/aegis/build/*"
# Source files
# Note: read_file.c is assumed based on read_file.h being present
SOURCES="Lexer.c tokenizer.c read_file.c"

# ==========================================
# Pre-build Checks
# ==========================================
# ==========================================
# Compilation
# ==========================================

echo "--- Compiling $APP_NAME ---"

# Run the compiler
$CC $CFLAGS -o $APP_NAME $SOURCES $LD

# Check if compilation succeeded
if [ $? -eq 0 ]; then
    echo "SUCCESS: Build complete."
    echo "Run with: ./$APP_NAME"
else
    echo "ERROR: Compilation failed."
    exit 1
fi
