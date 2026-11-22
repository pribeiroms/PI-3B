# Benchmark experimental da AGM — issue #77

O roteiro `scripts/benchmark_agm.ps1` executa o modo `--modo agm` cinco vezes
por subconjunto, separadamente para lista e matriz. Usa os oito subconjuntos
densos da Fase II da issue #70 (`N=10, 20, 30, 40, 100, 500, 1.000 e 5.000`),
com `--limite 0` para carregar cada arquivo inteiro. Os CSVs gerados ficam em
`results/agm_lista.csv` e `results/agm_matriz.csv`.

## Indicadores e interpretação

- `vertices` e `arestas` descrevem o grafo candidato de cabeamento construído
  antes de Kruskal.
- `tempo_parede_total_ms` mede o processo completo do modo AGM: leitura do CSV,
  construção do grafo candidato, Kruskal e detecção de cruzamentos no candidato
  e na floresta. Assim, inclui mais do que apenas o tempo interno de Kruskal.
- `memoria_estimada_bytes` é a estimativa de #86 para os blocos requisitados ao
  alocador pela estrutura do grafo candidato na representação selecionada.
  Não é RSS nem pico de memória do processo e não inclui buffers temporários,
  Union-Find, cópia ordenada de arestas ou a floresta de saída.
- `custo_agm_km` é a soma dos pesos das arestas escolhidas por Kruskal.

As cinco medições preservam cada repetição no CSV; agregações como média e
desvio padrão devem ser calculadas por `estrutura` e `vertices`, sem descartar
as observações individuais.

## Reprodução

```powershell
mingw32-make all
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/benchmark_agm.ps1 -Estrutura lista -Repeticoes 5 -Saida results/issue77-lista-SEUNOME.csv
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/benchmark_agm.ps1 -Estrutura matriz -Repeticoes 5 -Saida results/issue77-matriz-SEUNOME.csv
```

Substitua `SEUNOME` por um identificador curto antes de executar. Assim cada
colaborador grava arquivos próprios; o roteiro também recusa sobrescrever uma
saída existente. Rode os comandos em sequência e devolva os dois CSVs. Cada
linha inclui identificador da coleta, repetição, compilador, sistema e
processador. Não é necessário enviar os logs temporários da pasta `build/`.

Se o roteiro detectar uma falha, envie também o texto completo do erro e o
log indicado em `build/agm-...stderr.log` ou `build/agm-...stdout.log`. Não
edite os CSVs antes de devolvê-los.
