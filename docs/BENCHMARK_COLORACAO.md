# Benchmark da coloração gulosa e exata — issue #84

Compara a coloração gulosa (ordem natural e Welsh-Powell, #79/#80) com a
coloração exata por backtracking (#81) no grafo de interferência entre torres
(#69), sobre os subconjuntos densos da Fase II (#70): `N = 10, 20, 30, 40, 100,
500, 1.000 e 5.000`. Cada combinação de tamanho, estrutura (lista e matriz) e
algoritmo é executada `REPETICOES` vezes (padrão 5).

## Peças

- `src/benchmark_coloracao.c` → `build/benchmark_coloracao.exe`: faz **uma**
  medição. Carrega o subconjunto inteiro, constrói o grafo de interferência na
  estrutura pedida e cronometra só o algoritmo de coloração, com o cronômetro
  de alta resolução da #86. Valida a coloração obtida (`coloracao_valida`) e
  imprime o resultado em linhas `chave: valor`. Com `--saida ARQ.csv` também
  acrescenta o registro no CSV padrão da Fase II (`medicao_gravar_csv`).
- `scripts/benchmark_coloracao.ps1`: percorre tamanhos, estruturas, algoritmos
  e repetições e grava uma linha por execução.

## Reprodução

```powershell
mingw32-make benchmark-coloracao
mingw32-make benchmark-coloracao REPETICOES=5 LIMITE_SEGUNDOS=60
```

Ou direto pelo roteiro, escolhendo o arquivo de saída:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/benchmark_coloracao.ps1 -Repeticoes 5 -LimiteSegundos 60 -Saida results/fase2/coloracao/coloracao_SEUNOME.csv
```

`-Tamanhos`, `-Estruturas` e `-Algoritmos` restringem a varredura, por exemplo
`-Algoritmos exata -Tamanhos 10,20`. O roteiro recusa sobrescrever uma saída
existente. Sem `-Saida`, grava em `results/fase2/coloracao/coloracao_<data>.csv`.

Cada execução da exata que estoura o limite consome o limite inteiro, então a
coleta padrão (5 repetições, 60 s) leva cerca de uma hora e quinze minutos.

## Colunas do CSV

| Coluna | Significado |
| --- | --- |
| `benchmark_id`, `repeticao` | Identificador da coleta e número da repetição. |
| `algoritmo` | `coloracao_gulosa`, `coloracao_welsh_powell` ou `coloracao_exata`. |
| `estrutura`, `dataset` | Lista ou matriz e o subconjunto usado. |
| `vertices`, `arestas`, `grau_maximo` | Tamanho do grafo de interferência e seu Δ. |
| `tempo_ms` | Tempo decorrido só do algoritmo de coloração (sem leitura nem construção). |
| `memoria_grafo_bytes` | Estimativa da #86 para a estrutura do grafo em uso. |
| `memoria_auxiliar_bytes` | Vetores de heap do algoritmo: 3·V·`size_t` na gulosa, 6·V·`size_t` no pico de Welsh-Powell, V·`size_t` na exata (a pilha da recursão não entra). |
| `numero_cores` | Canais usados; vazio quando a exata foi interrompida. |
| `coloracao_valida` | `sim` se nenhuma aresta liga duas torres do mesmo canal. |
| `status` | `ok` ou `limite_tempo` (execução interrompida). |
| `limite_segundos` | Teto de tempo dado à exata. |
| `limite_inferior_exata` | Menor k não descartado: no `ok` é o número cromático; no `limite_tempo`, ficou provado que k−1 cores não bastam. |
| `nos_visitados_exata` | Nós da árvore de busca visitados pelo backtracking. |
| `compilador`, `sistema`, `processador` | Ambiente da coleta. |

## Leitura dos resultados

- A gulosa usa no máximo Δ+1 cores; comparar `numero_cores` com `grau_maximo`
  mostra quão perto do pior caso ela ficou.
- Quando a exata termina, `numero_cores` dela é o ótimo e a diferença para a
  gulosa é o custo da heurística. Quando é interrompida, `limite_inferior_exata`
  ainda dá um piso para o número de canais, e o intervalo
  `[limite_inferior_exata, cores da gulosa]` contém o ótimo.
- A exata é exponencial: nos testes iniciais com 5 s de limite ela só terminou
  em N = 10; a partir de N = 20 todas as execuções foram interrompidas.
- Agregações (média, desvio padrão) devem ser feitas por `algoritmo`,
  `estrutura` e `vertices`, sem descartar as repetições individuais.
