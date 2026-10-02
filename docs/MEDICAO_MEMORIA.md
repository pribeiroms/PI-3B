# Estimativa de memória das representações — issue #19

## Método

A aplicação calcula em bytes o tamanho solicitado ao alocador pelos blocos de
armazenamento do grafo. Ela apresenta um total para a lista e outro para a
matriz, além do armazenamento comum às duas representações. Os totais são
calculados sobre o mesmo conjunto de vértices e arestas:

- **Comum:** objeto `Grafo`, vetor de vértices na capacidade reservada e vetor
  de arestas na capacidade reservada.
- **Lista:** armazenamento comum, vetor de cabeças na capacidade reservada e
  dois nós `NoAdjacencia` por aresta não direcionada.
- **Matriz:** armazenamento comum e matriz simétrica compactada em bits; cada
  linha usa `ceil(ordem / 8)` bytes. Para uma ordem de 65.536, são 512 MiB,
  em vez de 4 GiB na representação anterior de um byte por célula.

As capacidades reservadas são usadas em vez da quantidade ocupada, pois são os
bytes que a aplicação efetivamente solicita nas alocações. As operações
verificam overflow; se uma soma ou multiplicação não couber em `size_t`, a
estimativa fica indisponível.

## Interpretação e limitações

Esta é uma estimativa reproduzível do tamanho dos blocos requisitados pela
aplicação, não uma leitura de RSS ou do pico de memória do processo. Não inclui
metadados e arredondamento do alocador, fragmentação, pilha, código carregado,
bufferização de I/O ou buffers temporários usados na construção/análise.
Consequentemente, não deve ser descrita no artigo como memória física total do
processo.

O modo `conjunta` mantém ambas as representações. Os modos `lista` e `matriz`
alocam apenas a estrutura selecionada, e o total da estrutura ausente é zero.
O campo `estrutura` identifica o modo efetivamente executado. Em modo exclusivo,
o total comum mais o total selecionado corresponde aos bytes estimados do grafo
naquela execução.

## Reprodução

Executar `mingw32-make all test` e, para gerar uma linha com um recorte do
dataset, `mingw32-make run`. A saída de terminal e as colunas
`memoria_comum_bytes`, `memoria_lista_total_bytes` e
`memoria_matriz_total_bytes` no CSV exibem a mesma estimativa. O estado
`estimada_modelo_alocacoes` identifica a metodologia.
