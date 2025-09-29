# === CONFIGURATION ===
CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -g $(shell pkg-config --cflags glib-2.0)
LDFLAGS := $(shell pkg-config --libs glib-2.0)

SRC_DIRS := Aeroki Lexer Parser libs/tools
OBJ_DIR  := build
BIN_DIR  := bin
TARGET   := $(BIN_DIR)/ark

# === FILE COLLECTION ===
SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# === RULES ===
.PHONY: all clean rebuild install run

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compile .c to .o with auto dep generation
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

rebuild: clean all

install:
	@echo "Installing to /usr/local/bin/ark"
	sudo cp $(TARGET) /usr/local/bin/ark

run: all
	./$(TARGET)


