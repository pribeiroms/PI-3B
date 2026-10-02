# Projeto Integrador 3B — Grafos em C

Aplicação em C para modelar uma rede de telecomunicações como grafo e analisar
a condição necessária de Euler e cruzamentos no traçado das conexões.

A aplicação local integra o fluxo principal da #34: carregar o dataset,
selecionar o limite de vértices e a estrutura (lista ou matriz), construir o
grafo, executar as análises, exibir e registrar os resultados. A validação
integrada da #35 passou em 02/10/2026 com o dataset real completo nos modos
lista e matriz. Benchmarks comparativos e a revisão da #36 ainda têm pendências
antes da entrega final.

## Estrutura

```text
include/   Interfaces públicas (`.h`)
src/       Implementações e ponto de entrada da aplicação
data/      Datasets de entrada versionados
tests/     Testes automatizados
results/   Resultados gerados por execuções (não versionados)
docs/      Documentação técnica e de entrega
```

## Pré-requisitos

- Ambiente validado: Windows com MinGW GCC 6.3.0 e `mingw32-make`.
- PowerShell para os roteiros de teste do fluxo e de integração.
- Python 3 para gerar e verificar os subconjuntos do protocolo experimental.
- Executar os comandos na raiz do projeto; os caminhos padrão são relativos a ela.

O código usa C11. O Makefile e os roteiros atuais contêm comandos específicos de
Windows; execução em outros sistemas ainda não foi validada.

## Compilação e execução

No terminal, na raiz do projeto:

```sh
mingw32-make
mingw32-make run
mingw32-make test
```

Para executar a modelagem geográfica da Fase I com o dataset versionado:

```sh
mingw32-make run
```

Consulte [docs/MODELAGEM_GRAFO.md](docs/MODELAGEM_GRAFO.md) para a regra de
conexão entre antenas e a interpretação do resultado de cruzamentos.

Os subconjuntos reproduzíveis para testes de estresse (N=100, 500, 1.000 e
5.000), suas contagens e a geração estão descritos em
[docs/PROTOCOLO_EXPERIMENTAL.md](docs/PROTOCOLO_EXPERIMENTAL.md). O benchmark
repetido da lista está em [docs/BENCHMARK_LISTA.md](docs/BENCHMARK_LISTA.md),
com dados brutos em `data/benchmarks/benchmark_lista.csv`.

Para executar novas repetições da lista e salvá-las em `results/`:

```sh
mingw32-make benchmark-lista
```

Os executáveis são gerados em `build/`. Para removê-los:

```sh
mingw32-make clean
```

No Windows sem `make`, compile diretamente com GCC:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude src/main.c src/grafo.c src/dataset.c src/analise_planaridade.c src/execucao.c -o grafo.exe -lm
./grafo.exe
```

O relatório consolidado da issue #17 e suas dependências estão em
[docs/ANALISE_PLANARIDADE.md](docs/ANALISE_PLANARIDADE.md).

O fluxo da #34 aceita opções e salva resultados em CSV:

```sh
./build/grafo.exe --limite 1000 --estrutura conjunta --saida results/execucoes.csv
```

Consulte [docs/FLUXO_APLICACAO.md](docs/FLUXO_APLICACAO.md) para uso,
validação, seleção entre estruturas, estimativa de memória e contagem de
cruzamentos.

A metodologia da estimativa de memória e suas limitações estão em
[docs/MEDICAO_MEMORIA.md](docs/MEDICAO_MEMORIA.md).

Para repetir a validação integrada completa da #35:

```sh
mingw32-make test-integracao
```

O teste completo gera logs, CSV e resumo em `results/integracao-<identificador>/`.
Consulte [docs/TESTE_INTEGRACAO_FASE1.md](docs/TESTE_INTEGRACAO_FASE1.md)
para os resultados observados e os limites desta validação.

## Revisão e entrega

A revisão da #36, os critérios de liberação e as pendências estão em
[docs/REVISAO_FASE1.md](docs/REVISAO_FASE1.md).
O [guia de entrega](docs/ENTREGA.md) organiza os documentos e a sequência final
de validação. Passar nos testes do fluxo disponível não conclui a Fase I.

## Contribuição

As regras de branches, commits, Pull Requests, revisão de código e convenções de
programação estão em [CONTRIBUTING.md](CONTRIBUTING.md).
