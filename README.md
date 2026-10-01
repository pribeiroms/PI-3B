# Projeto Integrador 3B — Grafos em C

Base inicial para o desenvolvimento autoral de estruturas de dados e algoritmos de grafos, atendendo ao RNF01 e aos requisitos de entrega do projeto.

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

- Compilador C compatível com C11 (GCC, Clang ou MinGW)
- `make` ou `mingw32-make` (opcional, recomendado)

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

Os executáveis são gerados em `build/`. Para removê-los:

```sh
mingw32-make clean
```

No Windows sem `make`, compile diretamente com GCC:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude src/main.c src/grafo.c src/dataset.c src/analise_planaridade.c src/execucao.c -o grafo.exe -lm
./grafo.exe
```

O relatório parcial da issue #17 e suas dependências estão em
[docs/ANALISE_PLANARIDADE.md](docs/ANALISE_PLANARIDADE.md).

O fluxo parcial da #34 aceita opções e salva resultados em CSV:

```sh
./build/grafo.exe --limite 1000 --estrutura conjunta --saida results/execucoes.csv
```

Consulte [docs/FLUXO_APLICACAO.md](docs/FLUXO_APLICACAO.md) para uso, validação
e dependências de seleção entre estruturas, memória e contagem de cruzamentos.

## Próximos passos

1. Definir a representação do grafo em `include/grafo.h` e `src/grafo.c`.
2. Implementar algoritmos autorais em novos módulos de `src/`.
3. Criar testes correspondentes em `tests/`.
4. Registrar datasets, metodologia e resultados em `data/`, `docs/` e `results/`.

## Contribuição

As regras de branches, commits, Pull Requests, revisão de código e convenções de
programação estão em [CONTRIBUTING.md](CONTRIBUTING.md).
