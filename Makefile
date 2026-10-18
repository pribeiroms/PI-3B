CC = gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude
LDFLAGS ?= -lm -lshell32

BUILD_DIR := build
APP := $(BUILD_DIR)/grafo.exe
TEST_APP := $(BUILD_DIR)/test_grafo.exe
OPERACOES_TEST_APP := $(BUILD_DIR)/test_operacoes_grafo.exe
CONEXOES_TEST_APP := $(BUILD_DIR)/test_conexoes_geograficas.exe
SRC := src/main.c src/grafo.c src/dataset.c src/analise_planaridade.c src/execucao.c src/exportar_dot.c src/utf8io.c
ANALISE_TEST_APP := $(BUILD_DIR)/test_analise_planaridade.exe
DOT_TEST_APP := $(BUILD_DIR)/test_exportar_dot.exe
TEST_SRC := tests/test_grafo.c src/grafo.c src/utf8io.c
OPERACOES_TEST_SRC := tests/test_operacoes_grafo.c src/grafo.c src/utf8io.c
CONEXOES_TEST_SRC := tests/test_conexoes_geograficas.c src/grafo.c src/utf8io.c
DATASET_TEST_SRC := tests/test_dataset.c src/dataset.c src/grafo.c src/utf8io.c
DATASET_TEST_APP := $(BUILD_DIR)/test_dataset.exe
DOT_TEST_SRC := tests/test_exportar_dot.c src/exportar_dot.c src/grafo.c src/utf8io.c
DOT_SAIDA ?= results/grafo.dot
PNG_SAIDA ?= results/grafo.png
DOT_MAPA ?= results/mapa.dot
PNG_MAPA ?= results/mapa.png
DPI ?= 150

.PHONY: all run dot png test test-fluxo test-integracao test-subconjuntos benchmark-lista clean

all: $(APP)

$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

$(APP): $(SRC) include/grafo.h include/dataset.h include/analise_planaridade.h include/execucao.h include/exportar_dot.h include/utf8io.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

$(TEST_APP): $(TEST_SRC) include/grafo.h include/utf8io.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $@ $(LDFLAGS)

$(OPERACOES_TEST_APP): $(OPERACOES_TEST_SRC) include/grafo.h include/utf8io.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OPERACOES_TEST_SRC) -o $@ $(LDFLAGS)

$(CONEXOES_TEST_APP): $(CONEXOES_TEST_SRC) include/grafo.h include/utf8io.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CONEXOES_TEST_SRC) -o $@ $(LDFLAGS)

$(DATASET_TEST_APP): $(DATASET_TEST_SRC) include/grafo.h include/dataset.h include/utf8io.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DATASET_TEST_SRC) -o $@ $(LDFLAGS)

$(DOT_TEST_APP): $(DOT_TEST_SRC) include/grafo.h include/exportar_dot.h include/utf8io.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DOT_TEST_SRC) -o $@ $(LDFLAGS)

run: $(APP)
	./$(APP)

# Gera apenas os arquivos .dot. Nao depende do Graphviz instalado: use a
# extensao Graphviz Interactive Preview do VS Code para visualizar.
dot: $(APP)
	./$(APP) --dot $(DOT_SAIDA) --dot-mapa $(DOT_MAPA)
	@echo Arquivos DOT gerados em $(DOT_SAIDA) e $(DOT_MAPA)
	@echo Abra-os no VS Code e use a preview com o motor neato.

# Converte os .dot em imagens. Exige o Graphviz instalado no sistema.
png: dot
	neato -Tpng -Gdpi=$(DPI) $(DOT_SAIDA) -o $(PNG_SAIDA)
	neato -Tpng -Gdpi=$(DPI) results/mapa_1_regiao.dot -o results/mapa_1_regiao.png
	neato -Tpng -Gdpi=$(DPI) results/mapa_2_noroeste.dot -o results/mapa_2_noroeste.png
	neato -Tpng -Gdpi=$(DPI) results/mapa_3_nordeste.dot -o results/mapa_3_nordeste.png
	neato -Tpng -Gdpi=$(DPI) results/mapa_4_sudoeste.dot -o results/mapa_4_sudoeste.png
	neato -Tpng -Gdpi=$(DPI) results/mapa_5_sudeste.dot -o results/mapa_5_sudeste.png
	@echo Imagens geradas em results/

test-fluxo: $(APP)
	powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_fluxo_parcial.ps1

test-integracao: test test-fluxo
	powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_integracao_fase1.ps1 -ExigirCompleto

test-subconjuntos: $(APP)
	powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_subconjuntos_estresse.ps1

benchmark-lista: $(APP)
	powershell -NoProfile -ExecutionPolicy Bypass -File scripts/benchmark_lista.ps1 -Saida results/benchmark_lista.csv

$(ANALISE_TEST_APP): tests/test_analise_planaridade.c src/analise_planaridade.c include/analise_planaridade.h include/grafo.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) tests/test_analise_planaridade.c src/analise_planaridade.c -o $@ $(LDFLAGS)

test: $(TEST_APP) $(OPERACOES_TEST_APP) $(CONEXOES_TEST_APP) $(DATASET_TEST_APP) $(ANALISE_TEST_APP) $(DOT_TEST_APP)
	./$(TEST_APP)
	./$(OPERACOES_TEST_APP)
	./$(CONEXOES_TEST_APP)
	./$(DATASET_TEST_APP)
	./$(ANALISE_TEST_APP)
	./$(DOT_TEST_APP)

clean:
	@cmd /C "if exist \"$(BUILD_DIR)\" rmdir /S /Q \"$(BUILD_DIR)\""