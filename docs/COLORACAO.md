# Algoritmos de coloração de vértices

## Objetivo e contexto

Este documento descreve os algoritmos de coloração implementados na Fase II
(Trilha C, issues #79 a #83) e serve de base para o artigo (Trilha E).

O problema vem do grafo de interferência (#69, descrito na seção 6 de
[MODELAGEM_PESOS_FASE2.md](MODELAGEM_PESOS_FASE2.md)): cada vértice é uma torre e
cada aresta liga duas torres da mesma operadora cujas áreas de cobertura se
sobrepõem. Atribuir canais de rádio de modo que torres vizinhas nunca dividam o
mesmo canal é exatamente colorir os vértices de modo que extremos de uma aresta
tenham cores diferentes. **Cada cor é um canal**, e o número de cores é o número
de canais necessários.

O menor número de cores possível é o **número cromático** χ(G). Calculá-lo é um
problema NP-difícil, por isso o projeto tem duas abordagens:

| Algoritmo | Tipo | Garante o mínimo? | Custo |
| --- | --- | --- | --- |
| Gulosa (ordem natural) | heurística | não | linear em V + E |
| Gulosa com Welsh-Powell | heurística | não | V log V + E |
| Backtracking | exata | sim | exponencial no pior caso |

Tudo está no módulo `coloracao` (`include/coloracao.h` e `src/coloracao.c`) e
funciona com as três estruturas do grafo: Lista, Matriz e estrutura conjunta.

## Estruturas de dados comuns

- **`ResultadoColoracao`**: vetor `cores[v]` com a cor de cada vértice, numerada
  a partir de 0, mais `quantidade_vertices` e `total_cores`. É o mesmo tipo para
  as três soluções, de modo que validação, mapeamento de canais e comparação
  tratam todas da mesma forma. É liberado com `coloracao_liberar`.
- **Acesso aos vizinhos**: na Lista (e na estrutura conjunta, que também mantém
  listas) os vizinhos de `v` são percorridos por `grafo_vizinhos`. Na Matriz não
  há lista de vizinhos; a adjacência é consultada par a par com
  `grafo_sao_adjacentes`, em tempo constante. O módulo decide qual caminho usar
  pelo getter `grafo_estrutura`, adicionado ao `grafo.h` na #79.
- **`SEM_COR`** (`SIZE_MAX`): marca vértice ainda não colorido.

## 1. Coloração gulosa (ordem natural)

### Funcionamento

Os vértices são visitados na ordem 0, 1, …, V−1. Para cada vértice:

1. marcam-se as cores dos vizinhos que já foram coloridos;
2. o vértice recebe a **menor cor não marcada**.

O resultado é sempre uma coloração válida, porque nenhum vértice recebe a cor de
um vizinho já colorido, e os vizinhos coloridos depois dele também o evitam.

Chamada: `coloracao_gulosa(grafo, &resultado)`.

### Estruturas de dados

- `cores[V]`: a resposta, iniciada com `SEM_COR`.
- `marca[V]`: vetor de "carimbos". Em vez de zerar um vetor de cores usadas a
  cada vértice, grava-se em `marca[cor]` o número do passo atual (`passo + 1`).
  Uma cor está ocupada só se a marca dela for a do passo atual. Isso evita uma
  limpeza de custo V por vértice, que deixaria o algoritmo quadrático.
- `sequencia[V]`: a ordem de visita (aqui, a identidade).

O vetor `marca` tem V posições porque um vértice nunca precisa de mais que V
cores: tem no máximo V−1 vizinhos, então alguma cor entre 0 e V−1 está livre.

### Complexidade

| | Lista / conjunta | Matriz |
| --- | --- | --- |
| Tempo | **O(V + E)** | **O(V²)** |
| Espaço extra | O(V) | O(V) |

Na Lista, marcar as cores dos vizinhos de `v` custa O(grau(v)), e a busca pela
menor cor livre para em no máximo grau(v) + 1 passos. Somando sobre todos os
vértices, isso dá O(V + E). Na Matriz, encontrar os vizinhos exige testar os V
vértices, o que dá O(V²) independentemente do número de arestas.

### Qualidade

A gulosa usa no máximo **Δ + 1 cores**, em que Δ é o grau máximo. É esse o
limite superior registrado pela modelagem da Fase II (coluna "Canais (Δ+1)").
Mas o resultado depende muito da ordem de visita, e pode ficar longe do ótimo:
no grafo "coroa" de 10 vértices (bipartido, χ = 2) a ordem natural usa **5
cores** (teste em `tests/test_coloracao_propriedades.c`).

### Decisões de implementação

- Ordem natural, como pede a #79: os índices dos vértices são a ordem de
  carregamento do CSV, então o resultado é reprodutível.
- Cores começam em 0 internamente; na conversão para canais, a cor `c` vira o
  canal `c + 1` (seção 4).

## 2. Gulosa com ordenação de Welsh-Powell

### Funcionamento

É a mesma coloração gulosa, mas os vértices são visitados em **ordem
decrescente de grau**. A intuição: os vértices mais restritos (com mais
vizinhos) escolhem cor primeiro, enquanto ainda há poucas cores em uso, e os de
grau baixo, que têm sobra de opções, ficam para o fim.

1. calcula-se o grau de cada vértice (`coloracao_calcular_graus`);
2. ordenam-se os vértices por grau decrescente (`coloracao_ordem_welsh_powell`);
3. executa-se a gulosa nessa ordem.

Chamada: `coloracao_gulosa_ordem(grafo, COLORACAO_WELSH_POWELL, &resultado)`.
`coloracao_comparar_ordens` executa as duas ordens e devolve o número de cores
de cada uma (`ComparacaoColoracao`).

### Estruturas de dados

- `graus[V]`: calculado percorrendo **o vetor de arestas** do grafo, e não as
  listas. Assim o cálculo é igual nas três estruturas e custa O(E) também na
  Matriz.
- `VerticeGrau[V]`: pares (vértice, grau) ordenados com `qsort`.
- Na coloração em si, as mesmas estruturas da seção 1.

### Complexidade

| | Lista / conjunta | Matriz |
| --- | --- | --- |
| Tempo | **O(V log V + E)** | **O(V²)** |
| Espaço extra | O(V) | O(V) |

O(V + E) para os graus, O(V log V) para a ordenação (caso típico do `qsort`) e o
custo da gulosa. Na Matriz domina o O(V²) da gulosa.

### Qualidade

Welsh-Powell continua com o limite de Δ + 1 cores e costuma usar menos cores
que a ordem natural, mas **não há garantia**: as duas são heurísticas. Exemplo
dos testes: no caminho 0–1–3–2–4 a ordem natural colore 0 e 2 primeiro com a
mesma cor, o que força o vértice 3 a usar uma terceira; Welsh-Powell visita
1, 2, 3, 0, 4 e resolve com 2 cores. Por isso a comparação é uma função
própria, e não uma suposição.

### Decisões de implementação

- **Empate pelo menor índice**: vértices de mesmo grau ficam na ordem natural.
  O `qsort` não é estável, então o desempate está no próprio comparador. Isso
  torna o resultado determinístico, o que importa para repetir experimentos.
- A ordem é escolhida por parâmetro (`OrdemColoracao`) em vez de uma função
  separada, para as duas versões compartilharem o mesmo código.

## 3. Coloração exata por backtracking

### Funcionamento

Para k = 1, 2, 3, …, tenta-se colorir o grafo com no máximo k cores. O primeiro
k que funciona é o número cromático, e a coloração encontrada é devolvida.

A tentativa com k cores é uma busca em profundidade: os vértices são coloridos
na ordem natural; para cada vértice testa-se cada cor permitida e desce-se para
o próximo vértice; se nenhuma cor serve, volta-se (*backtrack*) e troca-se a
cor do vértice anterior.

Duas podas reduzem a busca:

1. **Conflito com vizinho** (pedida na #81): uma cor já usada por um vizinho
   colorido não é testada, e o ramo inteiro abaixo dela é descartado.
2. **Quebra de simetria**: trocar o nome das cores de uma coloração dá outra
   coloração equivalente. Para não explorar as k! versões da mesma solução, o
   vértice `v` só pode receber uma cor já usada nos vértices anteriores ou a
   **próxima cor nova** (`cores_usadas`). O vértice 0 sempre recebe a cor 0, o
   primeiro vértice que precisar de outra recebe a 1, e assim por diante.

Chamada:

```c
LimitesColoracaoExata limites = {50U, 2.0};   /* max 50 vertices, max 2 s; 0 desliga */
InfoColoracaoExata info;
StatusColoracaoExata status = coloracao_exata(grafo, &limites, &resultado, &info);
puts(coloracao_mensagem_exata(status));
```

### Limites e interrupção

- `max_vertices`: grafos maiores não são processados (`COLORACAO_EXATA_LIMITE_TAMANHO`).
- `max_segundos`: a busca para quando o tempo passa do limite
  (`COLORACAO_EXATA_LIMITE_TEMPO`). Nesse caso o resultado não é preenchido.
- `InfoColoracaoExata` é preenchido mesmo com interrupção: tempo gasto, nós
  visitados e `limite_inferior`. Como cada k só é testado depois de k−1 falhar,
  ao interromper em k fica **provado que k−1 cores não bastam**. Esse limite
  inferior é útil no relatório mesmo sem o valor exato.

### Estruturas de dados

- `BuscaExata`: contexto da busca (grafo, k atual, vetor de cores, relógio,
  contador de nós e indicador de tempo esgotado), passado por ponteiro para a
  função recursiva em vez de usar variáveis globais.
- `cores[V]`: a coloração parcial. Como a ordem é natural, os vértices já
  coloridos são exatamente os de índice menor que `v`, e a verificação de
  conflito só olha esses.
- Pilha de recursão: um nível por vértice.

### Complexidade

| | Valor |
| --- | --- |
| Tempo (pior caso) | **exponencial**: até k^V folhas na tentativa com k cores, somando as tentativas de 1 a χ |
| Custo por nó | O(grau(v)) na Lista e na conjunta; O(V) na Matriz |
| Espaço extra | O(V) para as cores, mais O(V) de profundidade de recursão |

As podas não mudam o pior caso, mas reduzem muito a busca na prática: a quebra
de simetria corta até um fator k! e o conflito com vizinho elimina ramos
inteiros logo no início. Mesmo assim, o tempo cresce rápido: nos testes, o grafo
de Mycielski de 23 vértices (χ = 5) termina em cerca de 0,01 s, e o de 47
vértices (χ = 6) não termina em 50 ms (já com a prova de que 4 cores não
bastam). Por isso a solução exata se destina a grafos pequenos, como prevê a
#81, e os limites existem.

### Decisões de implementação

- **k crescente**, como pede a #81. Testar do menor para o maior é o que permite
  afirmar que o primeiro k encontrado é o mínimo e, em caso de interrupção, dar
  o limite inferior.
- **Ordem natural na busca.** Visitar os vértices em ordem de Welsh-Powell
  tenderia a podar mais cedo; fica como melhoria possível.
- **O resultado da gulosa não é usado como limite superior.** Seria possível
  parar a busca em k = (cores da gulosa) e devolver a gulosa, mas isso mistura
  as duas soluções e atrapalha a comparação de tempo pedida na #84.
- **Relógio consultado a cada 4.096 nós**, e não a cada nó, porque `clock()` tem
  custo. O atraso é pequeno: com limite de 50 ms a busca parou em 0,051 s nos
  testes. Observação para o artigo: `clock()` mede tempo de relógio no Windows e
  tempo de CPU no Linux.

## 4. Validação e mapeamento de canais (#82)

### Validação

`coloracao_validar` vale para o resultado de qualquer uma das três soluções e
preenche `ValidacaoColoracao`:

- `arestas_em_conflito` e `primeira_aresta_conflito`: percorre todas as arestas
  e conta as que ligam dois vértices da mesma cor;
- `cores_consistentes`: toda cor é menor que `total_cores` e todas as cores de 0
  a `total_cores − 1` são usadas, ou seja, `total_cores` é de fato o número de
  canais;
- `valida`: as duas condições acima.

`coloracao_valida` é o atalho que devolve só 1 ou 0. **Tempo O(V + E), espaço
O(total_cores).**

### Mapeamento de canais

`coloracao_mapear_canais` converte cada cor em um canal (`CanalFrequencia`) e
conta as torres de cada canal. A cor `c` vira o canal `c + 1`, ocupando a faixa

    [inicial + c × largura, inicial + (c + 1) × largura) MHz

A frequência inicial e a largura do canal são parâmetros (`PlanoFrequencias`),
porque a modelagem da Fase II não fixa valores em MHz: depende da faixa
licenciada de cada operadora. `coloracao_exibir_canais` imprime o relatório:

```
Distribuicao de canais de radio
Canais necessarios: 2
Canal 1: 2400.000 - 2405.000 MHz, 3 torre(s)
Canal 2: 2405.000 - 2410.000 MHz, 1 torre(s)
Total de torres: 4
```

**Tempo O(V + C), espaço O(C)**, em que C é o número de canais.

## 5. Testes

| Arquivo | O que cobre |
| --- | --- |
| `tests/test_coloracao.c` | Cores esperadas da gulosa nas duas ordens, graus e ordem de Welsh-Powell, número cromático de grafos clássicos (C5, K4, Petersen, Grötzsch), limites de tamanho e de tempo, validação de colorações erradas e mapeamento de canais |
| `tests/test_coloracao_propriedades.c` | Propriedades pedidas na #83: bipartidos com 2 cores, Kn com n cores, ciclos ímpares com 3 cores, e, em 240 grafos aleatórios por estrutura, gulosa sempre válida, com no máximo Δ + 1 cores e nunca menos cores que a exata |

Todos os testes rodam nas três estruturas (Lista, Matriz e conjunta) com
`make test`.

## 6. Resumo para o artigo

- A gulosa resolve o grafo de interferência em tempo linear na Lista e dá um
  número de canais **válido**, mas sem garantia de ser o mínimo; o limite
  teórico é Δ + 1.
- Welsh-Powell custa quase o mesmo (uma ordenação a mais) e costuma economizar
  canais, mas também sem garantia.
- O backtracking dá o **mínimo** de canais (número cromático), ao custo de tempo
  exponencial, e por isso é limitado a grafos pequenos; quando é interrompido,
  ainda informa um limite inferior.
- A diferença entre heurística e ótimo pode ser grande no pior caso (5 cores
  contra 2 no grafo coroa). Os números nos subconjuntos reais do dataset são
  medidos no benchmark (#84).
