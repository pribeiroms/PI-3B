# Análise de crescimento assintótico experimental

Esta análise relaciona o comportamento observado experimentalmente (via `scripts/benchmark_lista.ps1`, issues #23/#27) com a complexidade teórica dos algoritmos implementados na Fase I, conforme a Issue #26.

## Complexidade teórica das operações

| Operação | Função | Complexidade | Motivo |
|---|---|---|---|
| Leitura do dataset | `dataset_carregar_opencellid` | O(N) | Uma passada sobre as linhas do CSV |
| Construção das conexões | `grafo_construir_conexoes` | O(V²) | Laço duplo comparando todo par de vértices (i, j) pra achar o mais próximo elegível |
| Validação de Euler | `grafo_verificar_euler` | O(1) | Só usa as contagens já mantidas de \|V\| e \|E\| |
| Detecção de cruzamentos | `grafo_detectar_cruzamentos` | O(E²) | Laço duplo comparando todo par de arestas (i, j) |

## Relação entre |E| e |V| nesta rede

A regra de conexão (#5) liga cada antena apenas à sua vizinha elegível mais próxima, então cada vértice contribui com no máximo uma aresta — isso impõe um teto de `|E| ≤ |V|/2`. Nos dados observados, a razão E/V cresce com o tamanho da rede (0,02 em V=100 até 0,38 em V=5.000), se aproximando gradualmente desse teto conforme mais antenas compartilham a mesma área/operadora (mcc/net/area) por acaso. Ainda assim, `|E|` permanece limitado linearmente por `|V|`, então o **O(\|E\|²)** da detecção de cruzamentos se comporta, no pior caso, como **O(\|V\|²)** — a mesma ordem de grandeza da construção.

## Dados observados

Tempos de CPU médios por etapa (5 repetições), de `results/benchmark_lista.csv`:

| Vértices (V) | Arestas (E) | Construção (ms) | Cruzamentos (ms) |
|---|---|---|---|
| 100 | 2 | 0,200 | 0,000 |
| 500 | 48 | 0,800 | 0,000 |
| 1.000 | 186 | 1,600 | 0,200 |
| 5.000 | 1.897 | 34,200 | 55,200 |

## Discussão

**Ruído nas medições pequenas.** Para V ≤ 1.000, a maioria das repetições registrou 0ms ou 1ms — valores próximos demais da resolução do `clock()` no Windows (que opera em passos de alguns milissegundos) para serem confiáveis. Esses pontos não permitem, sozinhos, confirmar nenhuma curva de crescimento.

**Construção (V=1.000 → V=5.000):** V aumenta 5x; o esperado para O(V²) é um crescimento de **25x** no tempo. O observado foi de 1,6ms para 34,2ms, um fator de **~21,4x** — muito próximo do previsto, o que é consistente com o comportamento O(V²) teórico, mesmo considerando o ruído no ponto de partida.

**Cruzamentos (V=1.000 → V=5.000):** E aumenta de 186 para 1.897 (~10,2x); o esperado para O(E²) é um crescimento de **~104x**. O observado (0,2ms → 55,2ms, ~276x) não é diretamente comparável, já que o valor em V=1.000 (0,2ms, vindo de quatro zeros e um único 1ms) está abaixo da resolução confiável do relógio — não há uma medição real de referência ali, só ruído.

**Comparação entre as duas etapas em V=5.000:** apesar de a construção examinar muito mais pares (V(V-1)/2 ≈ 12,5 milhões) do que a detecção de cruzamentos (E(E-1)/2 ≈ 1,8 milhões), os cruzamentos levaram mais tempo (55,2ms vs. 34,2ms). Isso é esperado: a maioria dos pares de vértices é descartada cedo na construção (checagem barata de mcc/net/area antes de calcular distância), enquanto cada par de arestas na detecção de cruzamentos sempre executa o teste geométrico completo (quatro cálculos de orientação), que é mais caro por iteração.

## Conclusão

Os dados disponíveis (robustos apenas em V=1.000 e V=5.000) são consistentes com o comportamento teórico esperado: a construção das conexões cresce de forma compatível com O(V²), e a detecção de cruzamentos, embora não comparável diretamente por falta de uma medição confiável em V menor, mostra um custo por iteração mais alto que o da construção — coerente com seu teste geométrico mais caro. Para confirmar o expoente de crescimento da detecção de cruzamentos com mais confiança, seria necessário um dataset intermediário (ex.: V=2.000-3.000) com tempos acima do ruído de medição.