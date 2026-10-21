# Modelagem da Fase II

Documento de modelagem da Fase II (issue #71). Define como as variáveis do
dataset OpenCelliD são traduzidas em pesos e em relações de vizinhança nos dois
grafos da Fase II, quais parâmetros foram adotados e quais limitações a
modelagem assume.

Consolida as decisões das issues #65 (pesos e critérios), #68 (grafo candidato
de cabeamento), #69 (grafo de interferência) e #70 (subconjuntos). Serve de base
para a seção de modelagem do artigo.

Todos os números deste documento são medidos executando o programa sobre os
dados versionados, e podem ser reproduzidos com os comandos indicados em cada
seção.

## 1. Os dois grafos da Fase II

A Fase II trabalha com dois grafos distintos sobre o mesmo conjunto de antenas:

| Grafo | Vértices | Arestas | Peso | Algoritmo |
| --- | --- | --- | --- | --- |
| Cabeamento | antenas | rotas possíveis de fibra | distância em km | Kruskal (#74) |
| Interferência | antenas | pares cujos sinais se sobrepõem | sem peso | Coloração (#79) |

São grafos diferentes e não devem ser confundidos: duas antenas podem ser
candidatas a receber fibra sem interferirem entre si, e duas antenas podem
interferir sem que faça sentido ligá-las por cabo.

## 2. Os conjuntos de dados

Existem **dois conjuntos de subconjuntos** no repositório, e os dois continuam
válidos. A contagem de vértices e arestas de cada um está em
`CONTAGEM_SUBCONJUNTOS.md`.

### Dados antigos (Fase I)

`data/subconjuntos/opencellid_n{100,500,1000,5000}.csv`, sorteados
uniformemente do Brasil inteiro por `scripts/gerar_subconjuntos.py`, com semente
fixa. **Não foram alterados pela Fase II**: arquivos, hashes e contagens
originais seguem intactos e verificados por `tests/test_subconjuntos_estresse.ps1`.

### Dados novos (Fase II)

`data/subconjuntos/fase2/opencellid_denso_n{10,20,30,40,100,500,1000,5000}.csv`,
gerados por `scripts/gerar_subconjuntos_fase2.py`.

O motivo de existirem é medido, não estético. A amostra uniforme espalha as
antenas por milhares de quilômetros, e o critério de interferência exige
proximidade da ordem de quilômetros. O resultado é que, nos tamanhos pequenos
pedidos pela issue #70 para a coloração exata, **a amostra uniforme produz
grafos de interferência sem nenhuma aresta**:

| N | Arestas de interferência, dados antigos | Arestas de interferência, dados novos |
| --- | --- | --- |
| 10 | sem subconjunto | 16 |
| 20 | sem subconjunto | 73 |
| 30 | sem subconjunto | 191 |
| 40 | sem subconjunto | 315 |
| 100 | 2 | 1.026 |
| 1.000 | 297 | 22.277 |

Colorir um grafo sem arestas é trivial: uma cor basta. A coloração exata (#81)
não teria nada a resolver.

### Critério de geração dos dados novos

O procedimento é determinístico e não usa sorteio:

1. o território é dividido numa grade de células de 0,1 grau;
2. escolhe-se a célula com mais antenas, desempatando pela menor latitude e
   depois pela menor longitude;
3. tomam-se as N antenas mais próximas do centro dessa célula.

A célula vencedora no dataset é a de centro em latitude −23,55 e longitude
−46,65, com 2.820 antenas: a região central de São Paulo. Os subconjuntos são
aninhados por construção, porque "as N mais próximas" contém "as N−1 mais
próximas", e o raio coberto cresce de 0,22 km em N=10 a 9,00 km em N=5.000.

    python scripts/gerar_subconjuntos_fase2.py --check

## 3. Peso do grafo de cabeamento

**Peso da aresta (u, v) = distância geodésica entre as antenas, em quilômetros.**

- Fórmula de Haversine, raio terrestre de 6.371 km — a mesma já usada na
  Fase I, centralizada pela issue #66 em `grafo_distancia_km`.
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

## 4. Regra de elegibilidade do grafo de cabeamento

### O problema herdado da Fase I

A Fase I liga cada antena **apenas à antena elegível mais próxima**. Isso produz
um grafo **sem nenhum ciclo**: uma floresta.

A Árvore Geradora Mínima de uma floresta é a própria floresta. O Kruskal
executaria sem erro e devolveria exatamente o grafo de entrada, sem descartar
nenhuma aresta, porque não há rota redundante para descartar. O resultado seria
tecnicamente correto e sem significado para a pergunta de negócio.

### Decisão

A Fase II **não reaproveita o grafo da Fase I**. A issue #68 construiu um grafo
candidato próprio, com a regra:

- cada antena é ligada às **k = 6** antenas mais próximas;
- o raio é limitado a **50 km**, para excluir rotas irreais entre regiões
  distantes;
- a elegibilidade **não exige mesma operadora**: a fibra é infraestrutura
  física, e o traçado de um duto não depende de quem opera a antena;
- as arestas são não direcionadas e sem repetição; laços são rejeitados.

O grau de um vértice pode passar de k, porque ele também é escolhido por antenas
que o tenham entre as k mais próximas delas. É esse excedente que dá ciclos ao
grafo e rotas redundantes ao Kruskal.

    grafo.exe --modo candidato [--k N] [--raio-km X]

## 5. Calibração de k e do raio

`k` e o raio máximo são parâmetros, não constantes do problema. O indicador de
calibração é a **quantidade de componentes conexos**: muitos componentes
significam grafo fragmentado, e um grafo muito denso encarece o Kruskal sem
ganho de qualidade.

### Calibração original, sobre os dados antigos (#68)

Medida sobre as 1.000 primeiras linhas do CSV completo:

| k | raio (km) | arestas | componentes | antenas sem rota |
| --- | --- | --- | --- | --- |
| 2 | 50 | 1.275 | 95 | 14 |
| 3 | 50 | 1.867 | 44 | 14 |
| 4 | 50 | 2.422 | 32 | 14 |
| 6 | 50 | 3.568 | 27 | 14 |
| 8 | 50 | 4.729 | 26 | 14 |
| 4 | 100 | 2.437 | 25 | 7 |
| 8 | 100 | 4.759 | 19 | 7 |

Nesse conjunto, aumentar k tem retorno decrescente: de k = 2 para k = 4 os
componentes caem de 95 para 32, mas de k = 4 para k = 8 caem só de 32 para 26,
ao preço de dobrar as arestas. Quem governa o resto é o raio: só aumentando-o
para 100 km as 14 antenas isoladas caem para 7. A escolha inicial foi k = 4.

### Recalibração sobre os dados novos (#71)

A mesma varredura sobre `opencellid_denso_n1000.csv`:

| k | arestas | componentes | antenas sem rota |
| --- | --- | --- | --- |
| 2 | 1.327 | 82 | 0 |
| 3 | 1.942 | 17 | 0 |
| **6** | **3.750** | **1** | **0** |
| 8 | 4.970 | 1 | 0 |

Com k = 4 restariam 5 componentes. **Com k = 6 o grafo fica conexo**, e isso vale
para todos os tamanhos:

| N | arestas | componentes | antenas sem rota | fibra candidata |
| --- | --- | --- | --- | --- |
| 10 | 35 | 1 | 0 | 6,36 km |
| 20 | 72 | 1 | 0 | 10,85 km |
| 30 | 113 | 1 | 0 | 16,97 km |
| 40 | 146 | 1 | 0 | 19,08 km |
| 100 | 383 | 1 | 0 | 67,81 km |
| 500 | 1.902 | 1 | 0 | 289,54 km |
| 1.000 | 3.750 | 1 | 0 | 515,79 km |
| 5.000 | 18.987 | 1 | 0 | 2.905,79 km |

Por isso **k = 6 foi adotado como padrão**: sobre os dados da Fase II o Kruskal
passa a devolver uma árvore geradora mínima única, e não uma floresta, o que
responde à pergunta de negócio na forma em que ela foi feita — interligar as
antenas, no plural, em uma rede só.

A coluna "fibra candidata" é a soma de **todas** as rotas candidatas, não a
metragem da árvore: esta sai do Kruskal (#74) e será uma fração daquela.

### O raio é inerte nos dados novos

O raio de 50 km não tem efeito sobre os subconjuntos densos. Medido em
`opencellid_denso_n1000.csv`, os raios de 50 km, 5 km e 2 km produzem resultado
idêntico: 3.750 arestas e 1 componente. A razão é geométrica — o subconjunto
inteiro cabe num raio de 3,28 km, então o limite nunca dispara.

O parâmetro foi **mantido** mesmo assim, como salvaguarda para datasets
geograficamente espalhados, onde ele evita rotas irreais entre regiões
distantes. Mas qualquer análise de sensibilidade ao raio sobre os dados da
Fase II não produzirá variação alguma, e isso precisa ser dito em vez de
apresentado como estabilidade do modelo.

## 6. Regra de interferência

### O problema

Duas torres próximas transmitindo no mesmo canal de rádio degradam o sinal uma
da outra. O número de canais disponíveis é limitado, e a distribuição precisa
garantir que torres vizinhas nunca recebam o mesmo canal. Esse é exatamente o
problema de coloração de vértices: cada cor é um canal, e vértices adjacentes
não podem compartilhar cor.

### Decisão

**Duas torres interferem quando são da mesma operadora e a distância entre elas
é menor ou igual à soma dos alcances estimados:**

    d(u, v) <= range(u) + range(v)

Ou seja, quando as áreas de cobertura se sobrepõem. O campo `range` já existe no
dataset.

- Mesma operadora significa **mesmo `mcc`, mesmo `net` e mesma `area`**. É o
  mesmo predicado de elegibilidade já usado na Fase I em
  `grafo_construir_conexoes_ex`. A diferença entre as duas fases é o que se faz
  com ele: a Fase I guarda apenas a vizinha elegível mais próxima de cada
  antena, enquanto aqui **todas** as vizinhas elegíveis viram aresta —
  interferência não escolhe a mais próxima, quem se sobrepõe disputa o canal.
- O grafo é **não ponderado**: a relação é binária, as antenas interferem ou
  não. Usa as estruturas da Fase I sem alteração, e usa
  `grafo_adicionar_aresta`, não a versão ponderada.
- Arestas não direcionadas e sem repetição: o laço de pares visita cada par
  uma única vez.
- São registrados vértices, arestas e **grau máximo**. O grau máximo interessa
  porque um grafo de grau máximo Δ é sempre colorível com no máximo Δ + 1 cores
  pela heurística gulosa, o que dá um limite superior de canais para comparar
  com o resultado obtido.

<!-- -->

    grafo.exe --modo interferencia

### Por que operadoras diferentes não interferem

Frequência é recurso licenciado e exclusivo: cada operadora opera numa faixa de
espectro concedida pelo órgão regulador. Duas torres de operadoras distintas
nunca transmitem na mesma frequência, logo não podem interferir uma na outra,
por mais próximas que estejam. O conflito de canal só existe dentro da mesma
operadora, que precisa dividir a faixa que possui.

O efeito dessa decisão no resultado é grande, e foi medido sobre os dados novos:

| N | Critério | Arestas | Grau máximo |
| --- | --- | --- | --- |
| 1.000 | `mcc` + `net` | 68.601 | 456 |
| 1.000 | `mcc` + `net` + `area` | 22.277 | **137** |
| 5.000 | `mcc` + `net` | 574.414 | 2.164 |
| 5.000 | `mcc` + `net` + `area` | 187.522 | **368** |

Sem o campo `area` no critério, o grau máximo em N=1.000 sobe de 137 para 456 —
mais de trezentos vizinhos por antena que representariam conflitos inexistentes,
e um dimensionamento de canais mais de três vezes maior que o necessário.

Cabe registrar que o `area` (LAC, *Location Area Code*) é uma subdivisão
administrativa **dentro** da rede da própria operadora, não uma operadora
distinta: quem identifica a operadora são `mcc` e `net`. Incluí-lo no critério é
uma escolha pragmática, que mantém o predicado idêntico ao da Fase I e controla
o efeito dos outliers de `range` descritos na seção 8. Fisicamente, duas torres
da mesma operadora em LACs vizinhos *podem* interferir.

### Resultados medidos

Sobre os dados novos:

| N | Arestas | Grau máximo (Δ) | Canais (Δ+1) | Antenas sem interferência |
| --- | --- | --- | --- | --- |
| 10 | 16 | 4 | 5 | 1 |
| 20 | 73 | 9 | 10 | 2 |
| 30 | 191 | 16 | 17 | 2 |
| 40 | 315 | 20 | 21 | 4 |
| 100 | 1.026 | 37 | 38 | 3 |
| 500 | 9.204 | 116 | 117 | 15 |
| 1.000 | 22.277 | 137 | 138 | 22 |
| 5.000 | 187.522 | 368 | 369 | 53 |

Δ + 1 é o **pior caso** da coloração gulosa. Quantos canais são efetivamente
necessários sai da coloração (#79), e espera-se que seja bem menos.

## 7. Impacto sobre a análise de cruzamentos

A issue #76 reaplica o detector de cruzamentos da Fase I sobre a Árvore Geradora
Mínima. Essa análise responde se o traçado mais econômico em metragem é também
aceitável em custo de obra: cada cruzamento de cabos subterrâneos exige câmara
de passagem, escavação em dois níveis e licença específica.

A troca de conjunto de dados mudou esse quadro de forma substantiva. A mesma
análise da Fase I, sobre 1.000 antenas de cada conjunto:

| Conjunto | Arestas | Cruzamentos |
| --- | --- | --- |
| Dados antigos (amostra uniforme) | 186 | 7 |
| Dados novos (recorte denso) | 686 | 252 |

Em N=5.000 a diferença se mantém: 225 cruzamentos em 1.897 arestas nos dados
antigos, contra 1.725 em 3.446 nos novos — de 12% das arestas envolvidas em
cruzamento para 50%.

A conclusão da Fase I sobre planaridade, portanto, **era consequência da
amostragem e não da rede**: numa amostra espalhada pelo Brasil as rotas quase
não se tocam. Isso não invalida a Fase I, que mediu corretamente o que tinha; mas
significa que a resposta à pergunta de negócio da Fase I muda com o recorte, e os
dois números devem ser relatados lado a lado na análise de resultados (#98).

## 8. Limitações assumidas

### Distância geodésica é limite inferior

A distância de Haversine é a linha reta sobre a superfície terrestre. O traçado
real de um cabo acompanha vias, dutos e faixas de servidão, e é sempre maior. A
metragem obtida é um **limite inferior** da necessária.

### O campo `range` é estimado e tem outliers

`range` é uma estimativa derivada de observações, não um raio de cobertura
medido em campo. A regra de interferência é uma aproximação geométrica
razoável, não um modelo de propagação de rádio.

A distribuição é fortemente assimétrica, medida no dataset completo:

| Estatística | Valor |
| --- | --- |
| Mínimo | 500 m |
| Mediana | 1.000 m |
| Percentil 90 | 2.615 m |
| Percentil 99 | 11.518 m |
| Máximo | 36.586 m |
| Antenas acima de 10 km | 792 |

Uma antena com 36 km de alcance declarado vira um vértice de grau
desproporcional e puxa o Δ para cima sozinha. **Nenhum teto foi aplicado**, para
manter o grafo fiel ao dado bruto, mas o efeito deve ser mencionado ao
interpretar o grau máximo.

### O modelo ignora emissão fora da faixa

Nenhum transmissor é perfeito, e parte da energia escapa da faixa licenciada.
Na prática isso é tratado por bandas de guarda e limites regulatórios, não por
alocação de canal, e ignorá-lo é o padrão em modelos de atribuição de canais.
Ainda assim é uma simplificação.

### O dataset descartava antenas 5G

Até a correção feita na issue #70, o carregador convertia o campo `cell` com 32
bits. O identificador de célula do 5G NR tem 36 bits, e **671 das 797 antenas NR
do dataset eram descartadas em silêncio** como registro inválido — 84% da
geração mais nova, concentrada justamente nas áreas densas que a Fase II estuda.
Após a correção, as 62.604 antenas do dataset carregam e os registros inválidos
são zero.

Qualquer número da Fase I medido sobre o **dataset completo** antes dessa
correção está defasado. Os subconjuntos da Fase I não foram afetados: nenhum
deles continha antena NR com `cell` acima de 32 bits.

### A visualização geográfica satura

O grafo de interferência não pode ser desenhado de forma legível por
coordenadas reais nos tamanhos grandes. Em N=500 são 9.204 arestas de 1,13 km
médio dentro de uma área de 21,3 km², o que dá cerca de 10.400 km de linha
desenhada — aproximadamente duas vezes mais traço do que a figura tem pixels. O
desenho satura por quantidade de tinta, não por escolha de layout.

Medido em km de linha por km² de área:

| N | Arestas | km de linha / km² | Legível |
| --- | --- | --- | --- |
| 10 | 16 | 24 | sim |
| 20 | 73 | 72 | no limite |
| 40 | 315 | 194 | não |
| 500 | 9.204 | 489 | não |

A mesma densidade que torna o problema de coloração interessante é a que torna o
mapa ilegível. Para a Fase II, a visualização do grafo de interferência precisa
de outra abordagem (#89): colorir vértices pelo canal atribuído em vez de
desenhar todas as arestas, ou recortar algumas dezenas de antenas.

O grafo de cabeamento não tem esse problema: em N=500 são 1.902 arestas curtas,
e o desenho mostra a rede.

## 9. Parâmetros adotados

| Item | Decisão |
| --- | --- |
| Peso do cabeamento | distância de Haversine, em km, `double`, 4 casas decimais |
| Raio terrestre | 6.371 km, igual ao da Fase I |
| Grafo candidato | k = 6 vizinhos mais próximos |
| Raio máximo do candidato | 50 km (inerte nos dados densos; salvaguarda) |
| Operadora no cabeamento | não é exigida |
| Regra de interferência | d(u, v) <= range(u) + range(v) |
| Operadora na interferência | exigida: mesmo `mcc`, `net` e `area` |
| Peso na interferência | nenhum; grafo não ponderado |
| Teto no campo `range` | nenhum |
| Dados dos experimentos | `data/subconjuntos/fase2/` |

## 10. O que cada trilha pode assumir a partir daqui

- **Trilha B (#72–#78):** o grafo candidato tem pesos `double` em km, contém
  ciclos e, com k = 6 sobre os dados da Fase II, é **conexo em todos os
  tamanhos** — o Kruskal deve produzir uma árvore geradora mínima única. Sobre
  os dados antigos ele é desconexo, e nesse caso o resultado é uma floresta
  geradora mínima, uma árvore por componente. A #76 deve medir os cruzamentos do
  resultado.
- **Trilha C (#79–#85):** o grafo de interferência é não ponderado e tem grau
  máximo alto nos tamanhos grandes (Δ = 368 em N=5.000). Os tamanhos N=10 a 40
  existem para a coloração exata e já têm arestas suficientes para exercitá-la.
  O número de cores obtido é mapeado para faixas de frequência na #82.
- **Trilha D (#89):** a visualização do grafo de interferência precisa de
  abordagem diferente da geográfica, e o rótulo atual do `.dot` informa "0
  cruzamentos" nesse modo quando o correto seria "não calculado".
- **Trilha E (#93–#99):** este documento é a base da seção de modelagem do
  artigo. A tabela completa de contagens está em `CONTAGEM_SUBCONJUNTOS.md`.