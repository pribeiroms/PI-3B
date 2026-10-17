# Modelagem de pesos da Fase II

Documento de decisão da trilha A (issue #65). Define como as variáveis do
dataset OpenCelliD são traduzidas em pesos e em relações de vizinhança nos
grafos da Fase II. Orienta as trilhas B (Árvore Geradora Mínima) e C
(coloração de frequências).

## 1. Os dois grafos da Fase II

A Fase II trabalha com dois grafos distintos sobre o mesmo conjunto de antenas:

| Grafo | Vértices | Arestas | Peso | Algoritmo |
| --- | --- | --- | --- | --- |
| Cabeamento | antenas | rotas possíveis de fibra | distância em km | Kruskal (#74) |
| Interferência | antenas | pares cujos sinais se sobrepõem | sem peso | Coloração (#79) |

São grafos diferentes e não devem ser confundidos: duas antenas podem ser
candidatas a receber fibra sem interferirem entre si, e duas antenas podem
interferir sem que faça sentido ligá-las por cabo.

## 2. Peso do grafo de cabeamento

**Peso da aresta (u, v) = distância geodésica entre as antenas, em quilômetros.**

- Fórmula de Haversine, raio terrestre de 6.371 km — a mesma já usada na
  Fase I, centralizada pela issue #66.
- Tipo `double`, registrado com 4 casas decimais, o que resolve 0,1 m.

### Justificativa

A pergunta de negócio do estudo de caso é interligar as antenas com o **mínimo
de fibra ótica**. Fibra é especificada, comprada e instalada por metro, de modo
que a distância não é um substituto da grandeza de interesse: ela **é** a
grandeza de interesse. Minimizar a soma dos pesos da árvore geradora equivale,
diretamente, a minimizar a metragem de cabo do projeto.

### Alternativas descartadas

- **Custo monetário por trecho.** Seria o mais fiel, mas exigiria preços por
  metro, por tipo de terreno e por tipo de obra, ausentes do dataset.
- **Distância ponderada por obstáculos** (relevo, hidrografia, malha viária).
  Não há esses dados no OpenCelliD, e incorporá-los extrapola o escopo.
- **Inverso do alcance (`range`).** O alcance descreve a cobertura de rádio da
  antena, não o custo de passar cabo entre dois pontos.

### Limitação assumida

A distância geodésica é a linha reta sobre a superfície terrestre. O traçado
real de um cabo acompanha vias, dutos e faixas de servidão, e é sempre maior.
O valor obtido é, portanto, um **limite inferior** da metragem necessária.
Isso deve constar na análise de resultados (#98).

## 3. Regra de elegibilidade do grafo de cabeamento

### O problema herdado da Fase I

A Fase I liga cada antena **apenas à antena elegível mais próxima**. Na
execução padrão, isso produz 1.000 vértices e 637 arestas, **sem nenhum ciclo**:
o grafo resultante é uma floresta.

A Árvore Geradora Mínima de uma floresta é a própria floresta. O Kruskal
executaria sem erro e devolveria exatamente o grafo de entrada, sem descartar
nenhuma aresta, porque não há rota redundante para descartar. O resultado seria
tecnicamente correto e sem significado para a pergunta de negócio.

### Decisão

A Fase II **não reaproveita o grafo da Fase I**. A issue #68 constrói um grafo
candidato próprio, com a regra:

- cada antena é ligada às **k = 4** antenas mais próximas;
- o raio é limitado a **50 km**, para excluir rotas irreais entre regiões
  distantes;
- a elegibilidade **não exige mesma operadora**: a fibra é infraestrutura
  física, e o traçado de um duto não depende de quem opera a antena;
- as arestas são não direcionadas e sem repetição; laços são rejeitados.

Com k = 4 o grafo passa a conter ciclos, e o Kruskal tem rotas redundantes para
descartar.

### Parâmetros e calibração

`k` e o raio máximo são parâmetros, não constantes do problema. O indicador de
calibração é a **quantidade de componentes conexos** gerada, que a #68 deve
registrar:

- muitos componentes → o grafo está fragmentado; aumentar `k` ou o raio;
- poucos componentes e grafo muito denso → o Kruskal fica lento sem ganho de
  qualidade; reduzir `k`.

Quando houver mais de um componente, o Kruskal produzirá uma **floresta
geradora mínima**: uma árvore por componente. Isso é esperado e deve ser
relatado, não corrigido.

## 4. Regra de interferência

### O problema

Duas torres próximas transmitindo no mesmo canal de rádio degradam o sinal uma
da outra. O número de canais disponíveis é limitado, e a distribuição precisa
garantir que torres vizinhas nunca recebam o mesmo canal. Esse é exatamente o
problema de coloração de vértices: cada cor é um canal, e vértices adjacentes
não podem compartilhar cor.

### Decisão

**Duas torres interferem quando a distância entre elas é menor ou igual à soma
dos alcances estimados:**

    d(u, v) <= range(u) + range(v)

Ou seja, quando as áreas de cobertura se sobrepõem. O campo `range` já existe
no dataset.

- A interferência é considerada **apenas entre antenas da mesma operadora**
  (mesmo `mcc` e mesmo `net`). Operadoras distintas transmitem em faixas
  licenciadas distintas, e a alocação de canais é feita dentro da faixa de cada
  uma. Considerar interferência entre operadoras produziria um grafo mais denso
  que o real e um número de cores artificialmente alto.
- O grafo é **não ponderado**: a relação é binária, as antenas interferem ou
  não. Usa as estruturas da Fase I sem alteração.
- Arestas não direcionadas e sem repetição.
- Devem ser registrados vértices, arestas e **grau máximo**. O grau máximo
  interessa porque um grafo de grau máximo Δ é sempre colorível com no máximo
  Δ + 1 cores pela heurística gulosa, o que dá um limite superior para comparar
  com o resultado obtido.

### Limitação assumida

O campo `range` do OpenCelliD é uma estimativa derivada de observações, não um
raio de cobertura medido em campo. A regra é uma aproximação geométrica
razoável, não um modelo de propagação de rádio. Deve constar na análise de
resultados.

## 5. Impacto sobre a análise de cruzamentos

A issue #76 reaplica o detector de cruzamentos da Fase I (#15, #16) sobre a
Árvore Geradora Mínima. Essa análise responde se o traçado mais econômico em
metragem é também aceitável em custo de obra: cada cruzamento de cabos
subterrâneos exige câmara de passagem, escavação em dois níveis e licença
específica.

Como o grafo candidato da #68 é mais denso que o da Fase I, é esperado que a
contagem de cruzamentos mude. Os dois números devem ser relatados lado a lado
na análise de resultados: a metragem total da AGM e a quantidade de cruzamentos
nela. A solução mais curta em cabo não é necessariamente a mais barata no total.

## 6. Resumo das decisões

| Item | Decisão |
| --- | --- |
| Peso do cabeamento | distância de Haversine, em km, `double`, 4 casas decimais |
| Raio terrestre | 6.371 km, igual ao da Fase I |
| Grafo candidato | k = 4 vizinhos mais próximos, raio máximo de 50 km |
| Operadora no cabeamento | não é exigida |
| Regra de interferência | d(u, v) <= range(u) + range(v) |
| Operadora na interferência | exigida (mesmo mcc e net) |
| Peso na interferência | nenhum; grafo não ponderado |

## 7. O que cada trilha pode assumir a partir daqui

- **Trilha B (#72–#78):** o grafo candidato tem pesos `double` em km, contém
  ciclos e pode ter mais de um componente conexo. O Kruskal deve produzir uma
  floresta geradora mínima e a #76 deve medir os cruzamentos do resultado.
- **Trilha C (#79–#85):** o grafo de interferência é não ponderado e pode ter
  grau máximo alto em regiões densas. O número de cores obtido é mapeado para
  faixas de frequência na #82.
- **Trilha E (#95):** este documento é a base da seção de modelagem do artigo.