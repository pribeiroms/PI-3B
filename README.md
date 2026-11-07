# Projeto Integrador 3B — Grafos em C

Implementação autoral de estruturas de dados e algoritmos de grafos para o estudo de caso de Telecomunicações (Tema 8), atendendo ao RNF01 e aos requisitos de entrega do projeto.

## Objetivo

Modelar a disposição geográfica de antenas de celular e centrais telefônicas como um grafo (antenas = vértices, conexões = arestas) para responder, na Fase I, se a planta permite a passagem de cabos subterrâneos sem que duas linhas se cruzem — combinando a validação pela Fórmula de Euler com a detecção geométrica de cruzamentos entre conexões.

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

## Dataset utilizado

O projeto usa o [OpenCelliD](https://www.opencellid.org/), filtrado para o Brasil, versionado em [`data/opencellid_brasil_filtrado.csv`](data/opencellid_brasil_filtrado.csv) (62.604 registros). Detalhes de como o dataset foi escolhido e preparado estão em [docs/DATASET.md](docs/DATASET.md), e a regra de modelagem (o que vira vértice, o que vira aresta) está em [docs/MODELAGEM_GRAFO.md](docs/MODELAGEM_GRAFO.md).

## Formato dos dados

O CSV segue o formato do OpenCelliD, com cabeçalho `radio,mcc,net,area,cell,lat,lon,range,samples,...`:

| Coluna | Significado |
|---|---|
| `radio` | Tecnologia de rádio (GSM, UMTS, LTE etc.) |
| `mcc` | Código do país da operadora |
| `net` | Código da operadora (MNC) |
| `area` | Código da área/localização |
| `cell` | Identificador da célula/antena |
| `lat`, `lon` | Coordenadas geográficas da antena |
| `range` | Alcance estimado da antena, em metros |
| `samples` | Quantidade de amostras usadas para estimar a posição |

## Compilação

No terminal, na raiz do projeto:

```sh
mingw32-make
mingw32-make test
```

Os executáveis são gerados em `build/`. Para removê-los:

```sh
mingw32-make clean
```

No Windows sem `make`, compile diretamente com GCC:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude src/main.c src/grafo.c src/dataset.c -o grafo.exe -lm
```

## Execução

```sh
mingw32-make run
```

ou, após compilar manualmente com GCC:

```sh
./grafo.exe
```

### Opções disponíveis

O programa aceita dois argumentos opcionais, nessa ordem:

```sh
./build/grafo.exe [caminho-do-csv] [limite-de-registros]
```

- **caminho-do-csv**: caminho para o arquivo CSV a carregar. Padrão: `data/opencellid_brasil_filtrado.csv`.
- **limite-de-registros**: quantidade máxima de antenas a carregar. Padrão: `1000`.

Exemplo carregando 500 antenas de outro arquivo:

```sh
./build/grafo.exe data/meu_dataset.csv 500
```

### Exemplo de execução

```text
Leitura do dataset: 5.000 ms (Lista de Adjacencia)
Registros invalidos ignorados: 0
Construcao do grafo: 3.000 ms (Lista de Adjacencia)
Vertices: 1000
Arestas sem peso: 637
Validacao por Euler: 0.000 ms (Lista de Adjacencia)
A validacao de Euler foi aceita, mas e inconclusiva
Analise de cruzamentos: 0.000 ms (Lista de Adjacencia)
Resultado: existem cruzamentos; a planta exige isolamento ou novas rotas.
```

## Contribuição

As regras de branches, commits, Pull Requests, revisão de código e convenções de
programação estão em [CONTRIBUTING.md](CONTRIBUTING.md).