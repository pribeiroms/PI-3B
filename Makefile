CC = gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude
LDFLAGS ?=

BUILD_DIR := build
APP := $(BUILD_DIR)/grafo.exe
TEST_APP := $(BUILD_DIR)/test_grafo.exe
SRC := src/main.c src/grafo.c
TEST_SRC := tests/test_grafo.c src/grafo.c

.PHONY: all run test clean

all: $(APP)

$(BUILD_DIR):
	@cmd /C "if not exist \"$(BUILD_DIR)\" mkdir \"$(BUILD_DIR)\""

$(APP): $(SRC) include/grafo.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

$(TEST_APP): $(TEST_SRC) include/grafo.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $@ $(LDFLAGS)

run: $(APP)
	./$(APP)

test: $(TEST_APP)
	./$(TEST_APP)

clean:
	@cmd /C "if exist \"$(BUILD_DIR)\" rmdir /S /Q \"$(BUILD_DIR)\""
