# Contagem de vértices e arestas dos subconjuntos

<!-- ARQUIVO GERADO. Não edite à mão: rode `mingw32-make tabela-subconjuntos`. -->

Este arquivo responde ao último item da issue #70. A contagem de arestas não é
propriedade do arquivo CSV: depende de qual grafo se constrói sobre ele. Como a
Fase II tem dois grafos com critérios próprios, cada subconjunto rende uma linha
com várias colunas.

## Como ler as duas tabelas

São **dois conjuntos de dados diferentes**, e os dois continuam no repositório:

- **Dados antigos** — os subconjuntos sorteados uniformemente do Brasil, usados
  na Fase I. Nada neles foi alterado: os arquivos, os hashes e as contagens
  originais seguem intactos e verificados pelos testes.
- **Dados novos** — o recorte da região mais densa do dataset, criado para a
  Fase II porque a amostra uniforme produz grafos de interferência vazios nos
  tamanhos pequenos, o que tornaria trivial a coloração exata.

Cada tabela traz **todas** as colunas, inclusive as combinações cruzadas: os
grafos da Fase II medidos sobre os dados antigos, e a análise da Fase I medida
sobre os dados novos. Ou seja, o título da seção diz de qual **dado** a linha
veio, e o nome da coluna diz qual **análise** produziu o número.

## O que cada coluna é

As colunas de cabeamento e interferência usam os critérios definidos na issue
#65 e documentados em `MODELAGEM_PESOS_FASE2.md`. "Canais (D+1)" é o grau máximo
mais um, que é o limite superior de cores da coloração gulosa — ou seja, o pior
caso de canais de rádio necessários.

As colunas "Arestas Fase I" e "Cruzamentos Fase I" são a análise da Fase I —
conexão pela antena elegível mais próxima, e detecção de cruzamento de arestas.
Na tabela dos dados novos elas são a **Fase I refeita**, e a comparação entre as
duas tabelas mostra por que a conclusão original sobre planaridade era frágil:
na amostra uniforme os cruzamentos são raros porque as antenas estão a centenas
de quilômetros umas das outras; no recorte denso eles aparecem às centenas, com
um número de arestas parecido. A escolha da amostra, e não a rede em si,
determinava a resposta.

Os números são medidos executando o programa sobre cada subconjunto, nunca
copiados à mão, para a tabela não envelhecer em silêncio quando algum critério
mudar.

### Dados antigos — subconjuntos da Fase I (amostra uniforme do Brasil)

| N | Vertices | Arestas Fase I | Cruzamentos Fase I | Arestas cabeamento | Arestas interferencia | Grau maximo | Canais (D+1) | Sem interferencia |
|---|---|---|---|---|---|---|---|---|
| 100 | 100 | 2 | 0 | 152 | 2 | 1 | 2 | 96 |
| 500 | 500 | 48 | 0 | 1.487 | 57 | 7 | 8 | 416 |
| 1000 | 1.000 | 186 | 7 | 3.346 | 297 | 10 | 11 | 697 |
| 5000 | 5.000 | 1.897 | 225 | 18.353 | 6.610 | 45 | 46 | 2.071 |

### Dados novos — subconjuntos da Fase II (recorte da regiao mais densa)

| N | Vertices | Arestas Fase I | Cruzamentos Fase I | Arestas cabeamento | Arestas interferencia | Grau maximo | Canais (D+1) | Sem interferencia |
|---|---|---|---|---|---|---|---|---|
| 10 | 10 | 7 | 1 | 35 | 16 | 4 | 5 | 1 |
| 20 | 20 | 13 | 2 | 72 | 73 | 9 | 10 | 2 |
| 30 | 30 | 19 | 2 | 113 | 191 | 16 | 17 | 2 |
| 40 | 40 | 25 | 2 | 146 | 315 | 20 | 21 | 4 |
| 100 | 100 | 66 | 10 | 383 | 1.026 | 37 | 38 | 3 |
| 500 | 500 | 336 | 124 | 1.902 | 9.204 | 116 | 117 | 15 |
| 1000 | 1.000 | 686 | 252 | 3.750 | 22.277 | 137 | 138 | 22 |
| 5000 | 5.000 | 3.446 | 1.725 | 18.987 | 187.522 | 368 | 369 | 53 |
