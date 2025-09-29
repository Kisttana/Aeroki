# === Configuration ===
CC      := gcc-15
CXX     := g++-15

CCSTD   := -std=c23
CXXSTD   := -std=c++26

OPTIMIZE:= -O2
WALL    := -Wall -Wextra 

CFLAGS  := $(CCSTD) $(WALL) $(OPTIMIZE) 
CXXFLAGS:=  $(CXXSTD) $(WALL) $(OPTIMIZE) 

LDFLAGS := 

SRC_DIR := Aeroki
BUILD_DIR := build
BIN_DIR := bin

TARGET := $(BIN_DIR)/ark

C_SRCS := $(wildcard $(SRC_DIR)/*.c)
CPP_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SRCS)) \
        $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CPP_SRCS))

DEPS := $(OBJS:.o=.d)

# === Rules ===
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $^ -o $@ $(LDFLAGS)

# C source
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# C++ source
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

setup:
	
