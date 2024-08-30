CC = g++

ROOT = .
SRC_DIR = $(ROOT)/src
BUILD_DIR = $(ROOT)/build

SYSROOT = $(abspath ./sysroot)

GREEN = \033[0;32m
YELLOW = \033[0;33m
NC = \033[0m

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

.PHONY: all compiler clean reset

all: $(BUILD_DIR) compiler

compiler: $(OBJ_FILES)
	@echo "$(GREEN)Linking compiler$(NC)"
	@$(CC) -o $(ROOT)/ent $(OBJ_FILES) -g -fsanitize=address,undefined

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "$(GREEN)Compiling $@$(NC)"
	@$(CC) -c -o $@ $< -std=c++23 -DSYSROOT=\"$(SYSROOT)\" -g -fsanitize=address,undefined

clean:
	@clear
	@rm -rf $(ROOT)/ent $(BUILD_DIR)
	@echo "$(YELLOW)Clean complete$(NC)"

reset:
	@$(MAKE) clean
	@$(MAKE)

$(BUILD_DIR):
	@mkdir -p $@
