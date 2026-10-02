CC = gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude
LDFLAGS ?= -lm

BUILD_DIR := build
APP := $(BUILD_DIR)/grafo.exe
TEST_APP := $(BUILD_DIR)/test_grafo.exe
OPERACOES_TEST_APP := $(BUILD_DIR)/test_operacoes_grafo.exe
CONEXOES_TEST_APP := $(BUILD_DIR)/test_conexoes_geograficas.exe
SRC := src/main.c src/grafo.c src/dataset.c src/analise_planaridade.c src/execucao.c
ANALISE_TEST_APP := $(BUILD_DIR)/test_analise_planaridade.exe
TEST_SRC := tests/test_grafo.c src/grafo.c
OPERACOES_TEST_SRC := tests/test_operacoes_grafo.c src/grafo.c
CONEXOES_TEST_SRC := tests/test_conexoes_geograficas.c src/grafo.c
DATASET_TEST_SRC := tests/test_dataset.c src/dataset.c src/grafo.c
DATASET_TEST_APP := $(BUILD_DIR)/test_dataset.exe

.PHONY: all run test test-fluxo test-integracao test-subconjuntos clean

all: $(APP)

$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

$(APP): $(SRC) include/grafo.h include/dataset.h include/analise_planaridade.h include/execucao.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

$(TEST_APP): $(TEST_SRC) include/grafo.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $@ $(LDFLAGS)

$(OPERACOES_TEST_APP): $(OPERACOES_TEST_SRC) include/grafo.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OPERACOES_TEST_SRC) -o $@ $(LDFLAGS)

$(CONEXOES_TEST_APP): $(CONEXOES_TEST_SRC) include/grafo.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CONEXOES_TEST_SRC) -o $@ $(LDFLAGS)

$(DATASET_TEST_APP): $(DATASET_TEST_SRC) include/grafo.h include/dataset.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DATASET_TEST_SRC) -o $@ $(LDFLAGS)

run: $(APP)
	./$(APP)

test-fluxo: $(APP)
	powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_fluxo_parcial.ps1

test-integracao: test test-fluxo
	powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_integracao_fase1.ps1

test-subconjuntos: $(APP)
	powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_subconjuntos_estresse.ps1

$(ANALISE_TEST_APP): tests/test_analise_planaridade.c src/analise_planaridade.c include/analise_planaridade.h include/grafo.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) tests/test_analise_planaridade.c src/analise_planaridade.c -o $@ $(LDFLAGS)

test: $(TEST_APP) $(OPERACOES_TEST_APP) $(CONEXOES_TEST_APP) $(DATASET_TEST_APP) $(ANALISE_TEST_APP)
	./$(TEST_APP)
	./$(OPERACOES_TEST_APP)
	./$(CONEXOES_TEST_APP)
	./$(DATASET_TEST_APP)
	./$(ANALISE_TEST_APP)

clean:
	@cmd /C "if exist \"$(BUILD_DIR)\" rmdir /S /Q \"$(BUILD_DIR)\""
