# Benchmark da lista de adjacência — issue #23

## Método

Foram executadas cinco execuções independentes para cada subconjunto da
[issue #22](PROTOCOLO_EXPERIMENTAL.md), sempre com `--estrutura lista` e
`--limite 0`. O modo exclusivo aloca a lista encadeada e não aloca a matriz;
isso é conferido pelo teste do benchmark. Cada execução lê o dataset, constrói
as arestas, avalia a condição de Euler, conta os cruzamentos geométricos e
registra os resultados no CSV.

O CSV bruto [benchmark_lista.csv](../data/benchmarks/benchmark_lista.csv)
guarda as 20 execuções, sem agregação, incluindo V, E, Euler, cruzamentos,
memória estimada da estrutura e tempos por etapa. `total_cpu_ms` mede o tempo de
CPU dentro do programa, desde a criação do grafo até sua liberação. A resolução
é de milissegundos nesta compilação; por isso, tempos muito curtos podem
aparecer como zero. `tempo_parede_total_ms` mede a invocação completa do
processo pelo roteiro e inclui inicialização e encerramento do processo, então
não deve ser interpretado como tempo puro do algoritmo.

A memória registrada corresponde aos blocos solicitados ao alocador para os
dados comuns e a lista; não inclui overhead do alocador ou buffers temporários.
Ver [MEDICAO_MEMORIA.md](MEDICAO_MEMORIA.md) para a metodologia completa.

## Resultado resumido

Medianas de `total_cpu_ms` nas cinco repetições, com os resultados funcionais
observados nesta versão:

| N | Vértices | Arestas | CPU total mediana (ms) | Memória comum (bytes) | Lista total (bytes) | Euler | Pares de arestas cruzados |
| ---: | ---: | ---: | ---: | ---: | ---: | --- | ---: |
| 100 | 100 | 2 | 0 | 7.216 | 7.760 | inconclusivo | 0 |
| 500 | 500 | 48 | 1 | 25.648 | 28.464 | inconclusivo | 0 |
| 1.000 | 1.000 | 186 | 4 | 51.248 | 58.320 | inconclusivo | 7 |
| 5.000 | 5.000 | 1.897 | 154 | 409.648 | 472.768 | inconclusivo | 225 |

## Reprodução

Na raiz do projeto, compile e execute:

```powershell
mingw32-make all
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/benchmark_lista.ps1 `
  -Repeticoes 5 -Saida data/benchmarks/benchmark_lista_nova.csv
```

O roteiro valida a estrutura selecionada, contagens, memória e ausência de
registros inválidos antes de salvar o CSV. Ele se recusa a sobrescrever um
arquivo existente. Os números de tempo são específicos do equipamento e do
ambiente registrados em cada linha; a tabela acima resume somente a execução
arquivada neste repositório.
