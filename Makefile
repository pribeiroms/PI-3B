CC = gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude
LDFLAGS ?= -lm

BUILD_DIR := build
APP := $(BUILD_DIR)/grafo.exe
TEST_APP := $(BUILD_DIR)/test_grafo.exe
SRC := src/main.c src/grafo.c src/dataset.c
TEST_SRC := tests/test_grafo.c src/grafo.c
DATASET_TEST_SRC := tests/test_dataset.c src/dataset.c src/grafo.c
DATASET_TEST_APP := $(BUILD_DIR)/test_dataset.exe

.PHONY: all run test clean

all: $(APP)

$(BUILD_DIR):
	@cmd /C "if not exist \"$(BUILD_DIR)\" mkdir \"$(BUILD_DIR)\""

$(APP): $(SRC) include/grafo.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

$(TEST_APP): $(TEST_SRC) include/grafo.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $@ $(LDFLAGS)

$(DATASET_TEST_APP): $(DATASET_TEST_SRC) include/grafo.h include/dataset.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DATASET_TEST_SRC) -o $@ $(LDFLAGS)

run: $(APP)
	./$(APP)

test: $(TEST_APP) $(DATASET_TEST_APP)
	./$(TEST_APP)
	./$(DATASET_TEST_APP)

clean:
	@cmd /C "if exist \"$(BUILD_DIR)\" rmdir /S /Q \"$(BUILD_DIR)\""
