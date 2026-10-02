# Projeto Integrador 3B — Grafos em C

Aplicação em C para modelar uma rede de telecomunicações como grafo e analisar
a condição necessária de Euler e cruzamentos no traçado das conexões.

A aplicação local integra o fluxo principal: carregar o dataset,
selecionar o limite de vértices e a estrutura (lista ou matriz), construir o
grafo, executar as análises, exibir e registrar os resultados.

## Estrutura

```text
include/ Interfaces públicas (`.h`)
src/ Implementações e ponto de entrada da aplicação
data/ Datasets de entrada versionados
tests/ Testes automatizados
results/ Resultados gerados por execuções (não versionados)
docs/ Documentação técnica e de entrega
```

## Pré-requisitos

- Ambiente validado: Windows com MinGW GCC 6.3.0 e `mingw32-make`.
- PowerShell para os roteiros de teste do fluxo e de integração.
- Python 3 para gerar e verificar os subconjuntos do protocolo experimental.
- Executar os comandos na raiz do projeto; os caminhos padrão são relativos a ela.

O código usa C11. O Makefile e os roteiros atuais contêm comandos específicos de
Windows; execução em outros sistemas ainda não foi validada.

## Dataset utilizado

O projeto usa o [OpenCelliD](https://www.opencellid.org/), filtrado para o Brasil, versionado em [`data/opencellid_brasil_filtrado.csv`](data/opencellid_brasil_filtrado.csv) (62.604 registros). Veja [docs/DATASET.md](docs/DATASET.md) para como o dataset foi escolhido e preparado, e [docs/MODELAGEM_GRAFO.md](docs/MODELAGEM_GRAFO.md) para a regra de conexão entre antenas.

### Formato dos dados

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

O fluxo da aplicação aceita opções e salva resultados em CSV:

```sh
./build/grafo.exe --limite 1000 --estrutura conjunta --saida results/execucoes.csv
```

Consulte [docs/FLUXO_APLICACAO.md](docs/FLUXO_APLICACAO.md) para uso,
validação, seleção entre estruturas, estimativa de memória e contagem de
cruzamentos.

A metodologia da estimativa de memória e suas limitações estão em
[docs/MEDICAO_MEMORIA.md](docs/MEDICAO_MEMORIA.md).

### Exemplo de execução

```text
Dataset: data/opencellid_brasil_filtrado.csv
Limite de vertices: 1000 (0 = todos)
Estrutura: lista e matriz simultaneas (conjunta).
Registros invalidos ignorados: 0
Tempos de CPU em ms (-1 = indisponivel):
Leitura: 12.000
Construcao: 3.000
Euler: 0.000
Cruzamentos: 7.000
Detalhamento dos cruzamentos:
  Aresta 21 (antenas 76-77) x Aresta 62 (antenas 140-136)
  Aresta 481 (antenas 774-825) x Aresta 529 (antenas 848-874)
Total da execucao: 23.000
Memoria estimada em bytes (modelo das alocacoes do grafo):
Comum: 65632
Lista de adjacencia: 94208
Matriz de adjacencia: 1114208

Analise consolidada de planaridade
Vertices: 1000
Arestas: 637
Condicao de Euler: E <= 3V - 6 atendida; planaridade inconclusiva.
Quantidade de cruzamentos: 2
Conclusao: Foram detectados cruzamentos no tracado dos cabos; avaliar isolamento ou alteracao das rotas.
Limitacoes: a condicao de Euler, isoladamente, nao constitui um teste completo de planaridade. Cruzamentos no desenho atual nao provam, por si so, que o grafo seja nao planar.
Resultados acrescentados em: results/execucoes.csv
```

Para executar a validação integrada disponível:

```sh
mingw32-make test-integracao
```

O teste completo gera logs, CSV e resumo em `results/integracao-<identificador>/`.
Consulte [docs/TESTE_INTEGRACAO_FASE1.md](docs/TESTE_INTEGRACAO_FASE1.md)
para os resultados observados.

## Revisão e entrega

A revisão da #36 e os critérios de liberação estão em
[docs/REVISAO_FASE1.md](docs/REVISAO_FASE1.md).
O [guia de entrega](docs/ENTREGA.md) organiza os documentos e a sequência final
de validação.

## Contribuição

As regras de branches, commits, Pull Requests, revisão de código e convenções de
programação estão em [CONTRIBUTING.md](CONTRIBUTING.md).