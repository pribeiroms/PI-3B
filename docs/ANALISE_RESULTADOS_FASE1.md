# Análise dos resultados da Fase I — issue #33

## 1. Fontes de dados

Esta análise usa, sem gerar novos dados:

- `data/benchmarks/benchmark_lista.csv` — 5 repetições por tamanho de entrada
  (N = 100, 500, 1.000 e 5.000), estrutura `lista`, geradas pelo benchmark da
  issue #23 (ver `docs/BENCHMARK_LISTA.md`).
- `docs/MEDICAO_MEMORIA.md` — metodologia da estimativa de memória (issue #19).
- `docs/TESTE_INTEGRACAO_FASE1.md` — validação de vértices/arestas/Euler/
  cruzamentos para N = 1 e 1.000 (issue #35, parcial).

Os subconjuntos (`data/subconjuntos/opencellid_n*.csv`) são sempre os
primeiros N registros válidos do dataset original, na mesma ordem (issue #22).
O dataset completo tem 62.604 registros; nenhum dos experimentos abaixo o
processa por inteiro.

## 2. Resultados por tamanho de entrada

Médias das 5 repetições de cada N:

| N | Vértices | Arestas | Cruzamentos | Euler | Tempo total CPU (ms) | Tempo de cruzamentos (ms) | Memória comum (bytes) | Memória lista (bytes) |
|---|---|---|---|---|---|---|---|---|
| 100 | 100 | 2 | 0 | Inconclusivo | 0,4 | 0,0 | 7.216 | 7.760 |
| 500 | 500 | 48 | 0 | Inconclusivo | 1,0 | 0,0 | 25.648 | 28.464 |
| 1.000 | 1.000 | 186 | 7 | Inconclusivo | 5,8 | 1,4 | 51.248 | 58.320 |
| 5.000 | 5.000 | 1.897 | 225 | Inconclusivo | 155,0 | 113,6 | 409.648 | 472.768 |

(Valores de tempo são `total_cpu_ms` e `cruzamentos_cpu_ms` médios do CSV;
memória vem de `memoria_comum_bytes` e `memoria_lista_total_bytes`, idênticos
nas 5 repetições de cada N, como esperado de uma estimativa determinística.)

## 3. Crescimento do tempo de execução

O tempo total de CPU cresce de forma não linear com N: de 0,4 ms (N=100)
para 155,0 ms (N=5.000), um aumento de ~387 vezes para um N 50 vezes maior.
A etapa de **detecção de cruzamentos** é a que mais pesa: passa de
praticamente 0 ms em N≤500 para 113,6 ms em N=5.000, dominando o tempo total
nos maiores N (113,6 de 155,0 ms, ~73%).

Isso é esperado pela implementação: `grafo_detectar_cruzamentos` compara cada
par de arestas sem vértice em comum, um algoritmo O(E²). De N=1.000 para
N=5.000, as arestas cresceram ~10,2× (186 → 1.897) e o tempo de cruzamentos
cresceu ~81× (1,4 → 113,6 ms) — próximo da ordem quadrática esperada (10,2² ≈
104×), considerando que a medição em N=1.000 é pequena o suficiente para
sofrer ruído de resolução do relógio.

**Limitação de medição**: para N=100 e N=500, a maioria das repetições
registrou 0,000 ms em quase todas as etapas. Isso não significa tempo zero
real — é a resolução do relógio do Windows (`clock()`, tipicamente ~15 ms),
insuficiente para medir operações muito rápidas. Os tempos só se tornam
informativos a partir de N≈1.000.

A coluna `tempo_parede_total_ms` do CSV fica sempre perto de 1.020–1.030 ms,
praticamente constante e independente de N — isso sugere que ela mede
principalmente o overhead de inicialização do processo/script de benchmark
(ex.: carregar o PowerShell), não o tempo do algoritmo em si. Por isso esta
análise usa `total_cpu_ms` e os tempos por etapa, não o tempo de parede, como
indicador de desempenho.

## 4. Consumo de memória

A memória estimada cresce com N, como esperado — mais vértices e arestas
exigem mais alocação. O crescimento não é estritamente linear (ex.: a memória
comum por vértice adicional é ~46 bytes entre N=100 e N=500, mas ~90 bytes
entre N=1.000 e N=5.000). Isso é consistente com a metodologia descrita em
`docs/MEDICAO_MEMORIA.md`: a estimativa usa a **capacidade reservada**
(que cresce por duplicação, não por unidade), não a quantidade realmente
ocupada — então o valor reportado depende de em que ponto da duplicação de
capacidade a execução parou, não só da quantidade de dados.

Importante repetir a limitação já documentada: esta é uma estimativa dos
bytes solicitados ao alocador pelas estruturas do grafo, **não** o uso real
de memória do processo (RSS/pico) — não inclui overhead do alocador,
fragmentação, pilha ou buffers temporários. Não deve ser citada no artigo
como "memória total usada pelo programa".

## 5. Resposta à pergunta de negócio da Fase I

*"É possível conectar os pontos da rede sem que os cabos se cruzem e exijam
estruturas caras de isolamento?"*

Com os critérios de conexão geográfica usados (proximidade + alcance), a
resposta depende do tamanho da rede modelada:

- **N=100 e N=500**: nenhum cruzamento detectado — o traçado seria viável
  sem isolamento extra nesses recortes.
- **N=1.000**: 7 cruzamentos detectados.
- **N=5.000**: 225 cruzamentos detectados.

Ou seja, à medida que a rede cresce (mais antenas no mesmo recorte
geográfico, maior densidade de conexões), cruzamentos passam a existir e
crescem rapidamente — de 7 para 225 cruzamentos entre N=1.000 e N=5.000,
um aumento de ~32×, bem mais que proporcional ao crescimento de N (5×). Isso
indica que, para redes densas como as representadas pelo dataset completo
(62.604 registros), é esperado que o traçado subterrâneo *não* comporte
cabos sem cruzamento, exigindo pontos de isolamento ou rotas alternativas em
várias partes da rede.

A validação por Euler (E ≤ 3V − 6) ficou **inconclusiva em todos os N**
testados — a condição necessária foi satisfeita, mas Euler sozinho nunca
prova planaridade (como demonstrado no caso K₃,₃ nos testes unitários).
Isso confirma, na prática, por que o grupo adotou a detecção geométrica de
cruzamentos como critério principal, e não apenas a fórmula de Euler.

## 6. Limitações do modelo e da medição

- **Ordem dos subconjuntos**: os recortes N=100/500/1.000/5.000 são sempre os
  primeiros N registros do CSV original, na mesma ordem — não uma amostra
  aleatória nem geograficamente representativa. Resultados podem refletir a
  ordem de cadastro do OpenCelliD, não a densidade real da rede completa.
- **Resolução do relógio**: tempos de CPU para N≤500 não são confiáveis
  (ficam no limiar de resolução de `clock()`).
- **Tempo de parede não informativo**: dominado por overhead de processo,
  não pelo algoritmo.
- **Estimativa de memória**: mede capacidade alocada, não uso real de
  memória do processo; cresce em saltos (duplicação de capacidade), não de
  forma estritamente linear.
- **Critério de conexão**: cada vértice conecta apenas ao seu vizinho
  elegível mais próximo dentro do alcance combinado (não a todos os vizinhos
  elegíveis), conforme `docs/MODELAGEM_GRAFO.md` — decisão deliberada para
  evitar uma malha completa de cabos, pouco representativa de uma instalação
  economicamente viável.
- **Cobertura parcial**: o dataset completo (62.604 registros) nunca foi
  processado nos experimentos analisados aqui; os resultados são válidos para
  os recortes testados, não extrapolados automaticamente para a rede inteira.
- **Dependências em aberto**: conforme `docs/REVISAO_FASE1.md` (#36), ainda
  faltam o benchmark da estrutura matriz (#23/#24) para comparação, e a
  validação de integração completa (#35) — esta análise cobre apenas os
  dados de lista de adjacência disponíveis até o momento.

## 7. Comparação entre estruturas (lista vs. matriz)

As perguntas "qual estrutura consumiu mais memória?" e "qual estrutura
apresentou melhor comportamento?" exigem dados da estrutura **matriz**, que
ainda não foram gerados — o benchmark correspondente é escopo da issue #24,
registrada como pendente em `docs/REVISAO_FASE1.md`. Esta análise cobre
apenas a estrutura **lista de adjacência** (issue #23/#19). A comparação
direta entre as duas estruturas fica para quando os dados da #24 existirem.

## 8. Conclusão

Dentro do escopo validado, a Fase I confirma que o traçado de cabos sem
cruzamentos é viável apenas para recortes pequenos/esparsos da rede
(N≤500 neste dataset); a partir de N=1.000, cruzamentos aparecem e crescem
de forma mais que proporcional ao tamanho da entrada. A etapa computacional
mais custosa é a detecção de cruzamentos (O(E²)), que já domina o tempo de
execução em N=5.000 e deve ser o principal ponto de atenção de desempenho ao
tentar processar o dataset completo.
