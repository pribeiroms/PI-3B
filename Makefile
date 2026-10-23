CC = gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude
LDFLAGS ?= -lm -lshell32

BUILD_DIR := build
APP := $(BUILD_DIR)/grafo.exe
TEST_APP := $(BUILD_DIR)/test_grafo.exe
OPERACOES_TEST_APP := $(BUILD_DIR)/test_operacoes_grafo.exe
CONEXOES_TEST_APP := $(BUILD_DIR)/test_conexoes_geograficas.exe
SRC := src/main.c src/grafo.c src/dataset.c src/analise_planaridade.c src/execucao.c src/exportar_dot.c src/candidato_fibra.c src/interferencia.c src/utf8io.c
ANALISE_TEST_APP := $(BUILD_DIR)/test_analise_planaridade.exe
DOT_TEST_APP := $(BUILD_DIR)/test_exportar_dot.exe
TEST_SRC := tests/test_grafo.c src/grafo.c src/utf8io.c
OPERACOES_TEST_SRC := tests/test_operacoes_grafo.c src/grafo.c src/utf8io.c
CONEXOES_TEST_SRC := tests/test_conexoes_geograficas.c src/grafo.c src/utf8io.c
DATASET_TEST_SRC := tests/test_dataset.c src/dataset.c src/grafo.c src/utf8io.c
DATASET_TEST_APP := $(BUILD_DIR)/test_dataset.exe
DOT_TEST_SRC := tests/test_exportar_dot.c src/exportar_dot.c src/grafo.c src/utf8io.c
DISTANCIA_TEST_APP := $(BUILD_DIR)/test_distancia_geografica.exe
DISTANCIA_TEST_SRC := tests/test_distancia_geografica.c src/grafo.c src/utf8io.c
PONDERADAS_TEST_APP := $(BUILD_DIR)/test_arestas_ponderadas.exe
PONDERADAS_TEST_SRC := tests/test_arestas_ponderadas.c src/grafo.c src/utf8io.c
CANDIDATO_TEST_APP := $(BUILD_DIR)/test_candidato_fibra.exe
CANDIDATO_TEST_SRC := tests/test_candidato_fibra.c src/candidato_fibra.c src/grafo.c src/utf8io.c
INTERFERENCIA_TEST_APP := $(BUILD_DIR)/test_interferencia.exe
INTERFERENCIA_TEST_SRC := tests/test_interferencia.c src/interferencia.c src/grafo.c src/utf8io.c
COLORACAO_TEST_APP := $(BUILD_DIR)/test_coloracao.exe
COLORACAO_TEST_SRC := tests/test_coloracao.c src/coloracao.c src/grafo.c src/utf8io.c
PROPRIEDADES_TEST_APP := $(BUILD_DIR)/test_coloracao_propriedades.exe
PROPRIEDADES_TEST_SRC := tests/test_coloracao_propriedades.c src/coloracao.c src/grafo.c src/utf8io.c
DPI ?= 150

# Execucoes dos modos da Fase II. LIMITE e ESTRUTURA valem para os quatro
# alvos; EXTRA repassa opcoes avulsas ao programa, por exemplo
# EXTRA="--k 6 --raio-km 30" no modo candidato.
#
# Os padroes de --k e --raio-km NAO sao repetidos aqui de proposito: eles
# vivem em include/candidato_fibra.h e sao aplicados pelo proprio programa.
# Duplica-los no Makefile criaria duas fontes de verdade que silenciosamente
# discordariam na primeira vez que uma delas mudasse.
# DATASET aponta os alvos para um arquivo especifico, por exemplo
# DATASET=data/subconjuntos/fase2/opencellid_denso_n500.csv. Vazio usa o
# dataset padrao do programa, que e o CSV completo.
DATASET ?=
# O rotulo sai do nome do arquivo e entra no nome dos .dot, para que mapas de
# conjuntos diferentes nao se sobrescrevam: rodar o mesmo alvo nos dados da
# Fase I e nos da Fase II deixa os dois no disco, com nomes que se explicam.
ROTULO := $(if $(DATASET),$(basename $(notdir $(DATASET))),completo)
OPCAO_DATASET := $(if $(DATASET),--dataset $(DATASET),)

# O conjunto sai do caminho do dataset: fase2 e o recorte denso da Fase II,
# qualquer outra coisa e dado da Fase I -- tanto o CSV completo (que sem
# --limite entrega as 1.000 primeiras linhas, concentradas no Rio de Janeiro e
# usadas nos mapas da Fase I) quanto os subconjuntos de amostra uniforme.
CONJUNTO := $(if $(findstring subconjuntos/fase2,$(DATASET)),fase2,fase1)

# As tres pastas de topo de results/ correspondem ao que cada saida e no
# relatorio, nao so a qual dado produziu:
#
#   results/fase1/        a Fase I como foi entregue, com os dados dela
#   results/fase1_nova/   a analise da Fase I refeita sobre os dados novos
#   results/fase2/        os grafos da Fase II (cabeamento e interferencia)
#
# Por isso a planaridade e o unico caso que muda de pasta conforme o dado:
# rodar a analise da Fase I nos dados novos produz a "Fase I nova". Cabeamento
# e interferencia nao existiam na Fase I, entao seguem o conjunto de dados.
PASTA_PLANARIDADE := $(if $(filter fase2,$(CONJUNTO)),results/fase1_nova/planaridade,results/fase1/planaridade)
PASTA_INTERFERENCIA := results/$(CONJUNTO)/interferencia
PASTA_CANDIDATO := results/$(CONJUNTO)/candidato

# As pastas sao criadas na hora de gravar. python ja e exigido pelos alvos de
# subconjuntos, e makedirs aceita barra normal no Windows, o que evita a
# conversao de barras que o mkdir do cmd exigiria.
CRIAR_PASTA = @python -c "import os,sys; os.makedirs(sys.argv[1], exist_ok=True)"

DOT_SAIDA ?= $(PASTA_PLANARIDADE)/grafo_$(ROTULO).dot
PNG_SAIDA ?= $(PASTA_PLANARIDADE)/grafo_$(ROTULO).png
DOT_MAPA ?= $(PASTA_PLANARIDADE)/mapa_$(ROTULO).dot
PNG_MAPA ?= $(PASTA_PLANARIDADE)/mapa_$(ROTULO).png
BASE_MAPA := $(basename $(DOT_MAPA))
# Com um subconjunto escolhido o padrao passa a ser carregar o arquivo inteiro:
# manter 1000 truncaria o subconjunto em silencio. Passar LIMITE na linha de
# comando continua tendo precedencia.
LIMITE ?= $(if $(DATASET),0,1000)
ESTRUTURA ?= conjunta
EXTRA ?=
DOT_CANDIDATO ?= $(PASTA_CANDIDATO)/$(ROTULO).dot
DOT_CANDIDATO_MAPA ?= $(PASTA_CANDIDATO)/$(ROTULO)_zoom.dot
DOT_INTERFERENCIA ?= $(PASTA_INTERFERENCIA)/$(ROTULO).dot
DOT_INTERFERENCIA_MAPA ?= $(PASTA_INTERFERENCIA)/$(ROTULO)_zoom.dot

# Qual conjunto de subconjuntos os benchmarks usam. fase1 e a amostra uniforme
# do Brasil, usada na Fase I; fase2 e o recorte da regiao mais densa, usado nos
# experimentos da Fase II. Os dois convivem no repositorio.
SUBCONJUNTOS ?= fase1

# Tamanhos dos subconjuntos da Fase II e o prefixo do caminho deles. Os alvos
# por tamanho montam o nome do arquivo a partir destes dois.
TAMANHOS_FASE2 := 10 20 30 40 100 500 1000 5000
TAMANHOS_GRANDES := 100 500 1000 5000
MODELO_FASE2_N := data/subconjuntos/fase2/opencellid_denso_n

# Onde a contagem de vertices e arestas da issue #70 fica gravada.
TABELA_SUBCONJUNTOS ?= docs/CONTAGEM_SUBCONJUNTOS.md

.PHONY: all run dot png candidato interferencia dot-candidato dot-interferencia subconjuntos-fase2 subconjuntos-check tabela-subconjuntos tabela-subconjuntos-tela fase2-tudo fase1-nova ajuda test test-fluxo test-integracao test-subconjuntos benchmark-lista clean

all: $(APP)

# Lista os alvos. Sem linhas em branco de proposito: "echo" sem argumento
# imprime "ECHO is on." no cmd do Windows.
ajuda:
	@echo ---------------------------------------------------------------
	@echo BUILD E TESTES
	@echo "  mingw32-make                    compila o grafo.exe"
	@echo "  mingw32-make test               os 10 binarios de teste"
	@echo "  mingw32-make test-integracao    integracao da Fase I"
	@echo "  mingw32-make test-subconjuntos  hashes e contagens travados"
	@echo "  mingw32-make clean              apaga o build"
	@echo ---------------------------------------------------------------
	@echo DADOS
	@echo "  mingw32-make subconjuntos-fase2   regera os subconjuntos densos"
	@echo "  mingw32-make subconjuntos-check   confere os dois conjuntos"
	@echo "  mingw32-make tabela-subconjuntos  grava a contagem em docs/"
	@echo ---------------------------------------------------------------
	@echo NUMEROS NO TERMINAL, SEM GRAVAR MAPA
	@echo "  mingw32-make candidato-n1000      cabeamento, N=1000"
	@echo "  mingw32-make interferencia-n1000  interferencia, N=1000"
	@echo "  mingw32-make numeros-n1000        os dois de uma vez"
	@echo "  (N pode ser 10, 20, 30, 40, 100, 500, 1000 ou 5000)"
	@echo ---------------------------------------------------------------
	@echo MAPAS .DOT
	@echo "  mingw32-make fase2-n500      os dois grafos da Fase II, N=500"
	@echo "  mingw32-make fase2-tudo      todos os tamanhos"
	@echo "  mingw32-make fase1-nova-n1000  planaridade nos dados novos"
	@echo "  mingw32-make fase1-nova      planaridade em 100, 500, 1000, 5000"
	@echo "  mingw32-make dot             planaridade nos dados antigos"
	@echo "  mingw32-make png             converte os mapas da planaridade"
	@echo ---------------------------------------------------------------
	@echo BENCHMARK
	@echo "  mingw32-make benchmark-lista                     dados antigos"
	@echo "  mingw32-make benchmark-lista SUBCONJUNTOS=fase2  dados novos"
	@echo ---------------------------------------------------------------
	@echo "Para um arquivo fora da lista: DATASET=caminho/do/arquivo.csv"
	@echo "Variaveis: LIMITE, ESTRUTURA, EXTRA, SUBCONJUNTOS, DATASET"
	@echo ---------------------------------------------------------------

$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

$(APP): $(SRC) include/grafo.h include/dataset.h include/analise_planaridade.h include/execucao.h include/exportar_dot.h include/candidato_fibra.h include/interferencia.h include/utf8io.h | $(BUILD_DIR)
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

$(DISTANCIA_TEST_APP): $(DISTANCIA_TEST_SRC) include/grafo.h include/utf8io.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DISTANCIA_TEST_SRC) -o $@ $(LDFLAGS)

$(PONDERADAS_TEST_APP): $(PONDERADAS_TEST_SRC) include/grafo.h include/utf8io.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(PONDERADAS_TEST_SRC) -o $@ $(LDFLAGS)

$(CANDIDATO_TEST_APP): $(CANDIDATO_TEST_SRC) include/grafo.h include/candidato_fibra.h include/utf8io.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CANDIDATO_TEST_SRC) -o $@ $(LDFLAGS)

$(INTERFERENCIA_TEST_APP): $(INTERFERENCIA_TEST_SRC) include/grafo.h include/interferencia.h include/utf8io.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INTERFERENCIA_TEST_SRC) -o $@ $(LDFLAGS)

$(COLORACAO_TEST_APP): $(COLORACAO_TEST_SRC) include/grafo.h include/coloracao.h include/utf8io.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COLORACAO_TEST_SRC) -o $@ $(LDFLAGS)

$(PROPRIEDADES_TEST_APP): $(PROPRIEDADES_TEST_SRC) include/grafo.h include/coloracao.h include/utf8io.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(PROPRIEDADES_TEST_SRC) -o $@ $(LDFLAGS)

run: $(APP)
	./$(APP)

# Gera apenas os arquivos .dot da analise de planaridade da Fase I. Nao depende
# do Graphviz instalado: use a extensao Graphviz Interactive Preview do VS Code
# para visualizar.
#
# Com DATASET apontando para o recorte denso, este mesmo alvo produz a "Fase I
# nova": a analise da Fase I refeita sobre os dados da Fase II, que e o que
# permite o artigo final apresentar as duas fases sobre o mesmo dado.
dot: $(APP)
	$(CRIAR_PASTA) $(PASTA_PLANARIDADE)
	./$(APP) $(OPCAO_DATASET) --limite $(LIMITE) --dot $(DOT_SAIDA) --dot-mapa $(DOT_MAPA)
	@echo Arquivos DOT gerados em $(DOT_SAIDA) e $(DOT_MAPA)
	@echo Abra-os no VS Code e use a preview com o motor neato.

# Converte os .dot em imagens. Exige o Graphviz instalado no sistema.
png: dot
	neato -Tpng -Gdpi=$(DPI) $(DOT_SAIDA) -o $(PNG_SAIDA)
	neato -Tpng -Gdpi=$(DPI) $(BASE_MAPA)_1_regiao.dot -o $(BASE_MAPA)_1_regiao.png
	neato -Tpng -Gdpi=$(DPI) $(BASE_MAPA)_2_noroeste.dot -o $(BASE_MAPA)_2_noroeste.png
	neato -Tpng -Gdpi=$(DPI) $(BASE_MAPA)_3_nordeste.dot -o $(BASE_MAPA)_3_nordeste.png
	neato -Tpng -Gdpi=$(DPI) $(BASE_MAPA)_4_sudoeste.dot -o $(BASE_MAPA)_4_sudoeste.png
	neato -Tpng -Gdpi=$(DPI) $(BASE_MAPA)_5_sudeste.dot -o $(BASE_MAPA)_5_sudeste.png
	@echo Imagens geradas em $(PASTA_PLANARIDADE)

# --- Modos da Fase II ---------------------------------------------------
# Estes alvos dependem de $(APP), entao recompilam o programa sozinhos se o
# codigo mudou: nao ha como rodar um grafo.exe velho sem perceber.

# Grafo candidato de cabeamento (issue #68): relata vertices, arestas,
# componentes conexos e a soma das rotas candidatas.
candidato: $(APP)
	./$(APP) $(OPCAO_DATASET) --limite $(LIMITE) --estrutura $(ESTRUTURA) --modo candidato $(EXTRA)

# Grafo de interferencia entre torres (issue #69): relata vertices, arestas
# e o grau maximo, que limita a quantidade de canais da coloracao.
interferencia: $(APP)
	./$(APP) $(OPCAO_DATASET) --limite $(LIMITE) --estrutura $(ESTRUTURA) --modo interferencia $(EXTRA)

# Os dois alvos abaixo tambem gravam os mapas em DOT, como o alvo "dot" faz
# para a Fase I. O caminho passado em --dot-mapa e um molde: o programa grava
# <base>_1_regiao.dot ate <base>_5_sudeste.dot, e nao um arquivo com o nome
# da base. Como o alvo "dot", nao exigem o Graphviz instalado.
dot-candidato: $(APP)
	$(CRIAR_PASTA) $(PASTA_CANDIDATO)
	./$(APP) $(OPCAO_DATASET) --limite $(LIMITE) --estrutura $(ESTRUTURA) --modo candidato $(EXTRA) --dot $(DOT_CANDIDATO) --dot-mapa $(DOT_CANDIDATO_MAPA)
	@echo Mapas do grafo candidato gerados a partir de $(DOT_CANDIDATO) e $(DOT_CANDIDATO_MAPA)

dot-interferencia: $(APP)
	$(CRIAR_PASTA) $(PASTA_INTERFERENCIA)
	./$(APP) $(OPCAO_DATASET) --limite $(LIMITE) --estrutura $(ESTRUTURA) --modo interferencia $(EXTRA) --dot $(DOT_INTERFERENCIA) --dot-mapa $(DOT_INTERFERENCIA_MAPA)
	@echo Mapas do grafo de interferencia gerados a partir de $(DOT_INTERFERENCIA) e $(DOT_INTERFERENCIA_MAPA)

# O alvo "png" acima converte apenas os mapas da Fase I: os nomes
# results/mapa_*.dot estao fixos nele. Converter os mapas dos modos da Fase II
# exige generaliza-lo, o que e escopo da issue #89.

test-fluxo: $(APP)
	powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_fluxo_parcial.ps1

test-integracao: test test-fluxo
	powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_integracao_fase1.ps1 -ExigirCompleto

test-subconjuntos: $(APP)
	powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_subconjuntos_estresse.ps1

benchmark-lista: $(APP)
	powershell -NoProfile -ExecutionPolicy Bypass -File scripts/benchmark_lista.ps1 -Subconjuntos $(SUBCONJUNTOS)

# --- Subconjuntos da Fase II (issue #70) ------------------------------------
# Os subconjuntos da Fase I continuam sendo gerados por
# scripts/gerar_subconjuntos.py e nao sao tocados por nenhum alvo daqui.

# Regrava data/subconjuntos/fase2/. O criterio e deterministico: nao ha semente
# e rodar de novo produz os mesmos bytes.
subconjuntos-fase2:
	python scripts/gerar_subconjuntos_fase2.py

# Confere que os dois conjuntos no disco correspondem ao dataset e ao criterio.
subconjuntos-check:
	python scripts/gerar_subconjuntos.py --check
	python scripts/gerar_subconjuntos_fase2.py --check

# Mede a contagem de vertices e arestas que a issue #70 pede e grava em
# docs/. A tabela e medida executando o programa, nao copiada a mao: regenerar
# e um comando, e o arquivo nunca fica desatualizado em silencio.
tabela-subconjuntos: $(APP)
	python scripts/tabela_subconjuntos.py --saida $(TABELA_SUBCONJUNTOS)

# Mesma tabela, so no terminal, sem gravar.
tabela-subconjuntos-tela: $(APP)
	python scripts/tabela_subconjuntos.py

# --- Mapas por subconjunto -------------------------------------------------
# Evitam digitar o caminho do dataset. Para um tamanho so:
#
#   mingw32-make fase2-n500        cabeamento e interferencia do n500
#   mingw32-make fase1-nova-n1000  planaridade do n1000 nos dados novos
#
# e para todos de uma vez, fase2-tudo e fase1-nova.
#
# Cada alvo chama o make de novo passando DATASET, em vez de repetir a receita:
# assim existe um unico lugar que sabe montar a linha de comando do programa.
# So os numeros no terminal, sem gravar mapa:
#
#   mingw32-make candidato-n1000
#   mingw32-make interferencia-n1000
candidato-n%:
	@$(MAKE) --no-print-directory candidato DATASET=$(MODELO_FASE2_N)$*.csv

interferencia-n%:
	@$(MAKE) --no-print-directory interferencia DATASET=$(MODELO_FASE2_N)$*.csv

# Os numeros dos dois grafos, um embaixo do outro.
numeros-n%:
	@$(MAKE) --no-print-directory candidato DATASET=$(MODELO_FASE2_N)$*.csv
	@$(MAKE) --no-print-directory interferencia DATASET=$(MODELO_FASE2_N)$*.csv

# Com os mapas .dot, alem dos numeros.
fase2-n%:
	@$(MAKE) --no-print-directory dot-interferencia DATASET=$(MODELO_FASE2_N)$*.csv
	@$(MAKE) --no-print-directory dot-candidato DATASET=$(MODELO_FASE2_N)$*.csv

fase1-nova-n%:
	@$(MAKE) --no-print-directory dot DATASET=$(MODELO_FASE2_N)$*.csv

# Todos os tamanhos. Os pequenos existem para a coloracao exata e sao os unicos
# em que o desenho da interferencia fica legivel; os grandes saturam o mapa.
fase2-tudo: $(addprefix fase2-n,$(TAMANHOS_FASE2))
	@echo Mapas da Fase II gerados em results/fase2/

# A planaridade so e rodada nos tamanhos grandes: com 10 ou 20 antenas a
# analise de cruzamentos nao diz nada util.
fase1-nova: $(addprefix fase1-nova-n,$(TAMANHOS_GRANDES))
	@echo Analise da Fase I sobre os dados novos gerada em results/fase1_nova/

$(ANALISE_TEST_APP): tests/test_analise_planaridade.c src/analise_planaridade.c include/analise_planaridade.h include/grafo.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) tests/test_analise_planaridade.c src/analise_planaridade.c -o $@ $(LDFLAGS)

test: $(TEST_APP) $(OPERACOES_TEST_APP) $(CONEXOES_TEST_APP) $(DATASET_TEST_APP) $(ANALISE_TEST_APP) $(DOT_TEST_APP) $(DISTANCIA_TEST_APP) $(PONDERADAS_TEST_APP) $(CANDIDATO_TEST_APP) $(INTERFERENCIA_TEST_APP) $(COLORACAO_TEST_APP) $(PROPRIEDADES_TEST_APP)
	./$(TEST_APP)
	./$(OPERACOES_TEST_APP)
	./$(CONEXOES_TEST_APP)
	./$(DATASET_TEST_APP)
	./$(ANALISE_TEST_APP)
	./$(DOT_TEST_APP)
	./$(DISTANCIA_TEST_APP)
	./$(PONDERADAS_TEST_APP)
	./$(CANDIDATO_TEST_APP)
	./$(INTERFERENCIA_TEST_APP)
	./$(COLORACAO_TEST_APP)
	./$(PROPRIEDADES_TEST_APP)

clean:
	@cmd /C "if exist \"$(BUILD_DIR)\" rmdir /S /Q \"$(BUILD_DIR)\""