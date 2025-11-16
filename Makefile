# HAL64 build system

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
TEST_BIN_DIR := $(BUILD_DIR)/tests
UNITY_DIR := libs/Unity

CFLAGS ?= -Iinclude -I$(UNITY_DIR)/src -std=c23 -Wall -Wextra -Wpedantic -O3

CORE_SRCS := \
	src/hal64.c \
	src/assembler/assemble.c \
	src/assembler/reader.c \
	src/lexer.c \
	src/utils/memory.c \
	src/vm.c

APP_SRCS := main.c
UNITY_SRC := $(UNITY_DIR)/src/unity.c

CORE_OBJS := $(CORE_SRCS:%.c=$(OBJ_DIR)/%.o)
APP_OBJS := $(APP_SRCS:%.c=$(OBJ_DIR)/%.o)
UNITY_OBJ := $(UNITY_SRC:%.c=$(OBJ_DIR)/%.o)

TEST_ASSEMBLER_OBJ := $(OBJ_DIR)/test/assembler.o $(OBJ_DIR)/test/string_reader.o
TEST_LEXER_OBJ := $(OBJ_DIR)/test/lexer.o $(OBJ_DIR)/test/string_reader.o

APP_BIN := $(BIN_DIR)/hal64
TEST_ASSEMBLER_BIN := $(TEST_BIN_DIR)/assembler
TEST_LEXER_BIN := $(TEST_BIN_DIR)/lexer
TEST_BINS := $(TEST_ASSEMBLER_BIN) $(TEST_LEXER_BIN)

DEPS := \
	$(CORE_OBJS:.o=.d) \
	$(APP_OBJS:.o=.d) \
	$(UNITY_OBJ:.o=.d) \
	$(TEST_ASSEMBLER_OBJ:.o=.d) \
	$(TEST_LEXER_OBJ:.o=.d)

.PHONY: all clean test run

all: $(APP_BIN)

$(APP_BIN): $(CORE_OBJS) $(APP_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(TEST_ASSEMBLER_BIN): $(TEST_ASSEMBLER_OBJ) $(CORE_OBJS) $(UNITY_OBJ)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(TEST_LEXER_BIN): $(TEST_LEXER_OBJ) $(OBJ_DIR)/src/lexer.o $(OBJ_DIR)/src/assembler/reader.o $(UNITY_OBJ)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

test: $(TEST_BINS)
	@set -e; \
	for t in $^; do \
		echo "Running $$t"; \
		$$t; \
	done

run: $(APP_BIN)
	$(APP_BIN) $(ARGS)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
