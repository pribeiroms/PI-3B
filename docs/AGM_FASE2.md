# Árvore geradora mínima da Fase II

Este documento descreve a solução das issues #72–#76 e #78. A AGM é calculada
sobre o grafo candidato de fibra da Fase II, cujas arestas têm peso em
quilômetros. Para grafos desconexos, o resultado é uma floresta geradora mínima.

## Conjuntos disjuntos (Union-Find)

`ConjuntosDisjuntos` mantém um vetor de pais e um vetor de ranks, com um
elemento inicial por vértice. `find` percorre até a raiz e comprime o caminho
na volta. `union` liga a raiz de menor rank à de maior rank; em empate, escolhe
uma raiz e aumenta seu rank. A API oferece criação, inicialização, consulta,
união e liberação da memória.

Com compressão de caminho e união por rank, as operações têm custo amortizado
O(α(V)), onde α é a inversa da função de Ackermann. O espaço é O(V). Os vetores
são liberados tanto no caminho normal quanto em falha de alocação.

## Extração e ordenação de arestas

`agm_extrair_arestas_ordenadas` copia o vetor interno de arestas do grafo e o
ordena com `qsort`, em O(E log E). O vetor interno comum às implementações em
lista, matriz e estrutura conjunta evita percorrer as duas representações e
garante o mesmo resultado em qualquer uma delas. O grafo é não direcionado e
recusa arestas repetidas na inserção, então cada conexão aparece uma vez.
Empates são desfeitos por origem e destino para manter a ordem determinística.
O chamador libera o vetor retornado com `free`.

Espaço adicional: O(E) para a cópia ordenada.

## Kruskal e custo da fibra

`agm_kruskal` percorre as arestas em ordem crescente. Uma aresta entra na
floresta quando une dois conjuntos distintos; ciclos são descartados. A
floresta preserva todos os vértices e copia os pesos das arestas selecionadas.
O relatório informa vértices, arestas, quantidade de árvores e custo total em
quilômetros. Cada componente conexo, inclusive um vértice isolado, corresponde
a uma árvore. Para grafo conexo com V vértices, são selecionadas V − 1 arestas.

O custo dominante é O(E log E + E α(V)); a memória adicional é O(V + E), sem
contar as estruturas próprias do grafo de saída. Grafos vazios produzem zero
árvores. Não há ligação artificial entre componentes desconexos.

## Comparação de cruzamentos e isolamento

O modo `--modo agm` constrói o grafo candidato, calcula sua floresta mínima e
aplica o detector existente de cruzamentos aos dois grafos. A saída compara as
quantidades e informa se a seleção reduziu cruzamentos ou se ainda há pontos
em que os cabos devem ser avaliados para isolamento. A análise geométrica
ignora pares de arestas que compartilham vértice, como no detector da Fase I.

O detector existente compara pares de arestas, portanto custa O(E²) no número
de arestas do grafo analisado. Essa etapa pode dominar o tempo em candidatos
grandes. Os mapas opcionais da AGM recebem os cruzamentos selecionados e os
destacam usando o exportador já existente.

## Uso

```text
grafo.exe --modo agm [--dataset arquivo.csv] [--limite N]
          [--estrutura lista|matriz|conjunta] [--k N] [--raio-km X]
          [--dot agm.dot] [--dot-mapa agm.dot]
```

`--k` e `--raio-km` usam os mesmos padrões e validações do modo `candidato`.
O resultado responde à pergunta de negócio: a fibra mínima necessária é a
soma dos pesos em km das arestas selecionadas. A medição representa a distância
geodésica dos enlaces candidatos, conforme o modelo da Fase II; não inclui
desvios de instalação ou outras restrições físicas.

## Decisões

- Union-Find fica encapsulado no módulo `agm`, evitando alterações à estrutura
  geral de grafo e reutilizando-o diretamente nos testes.
- A ordenação usa as arestas ponderadas já mantidas pelo grafo, que preserva
  um vetor de arestas em todas as estruturas de adjacência.
- Kruskal retorna um grafo independente da mesma estrutura do candidato, para
  que o detector geométrico e o exportador de mapas existentes possam ser
  reutilizados sem alterar a Fase I.
- Pesos continuam em quilômetros e somas não finitas são tratadas como erro.
- O modo de execução é opt-in (`--modo agm`); os modos Fase I, candidato e
  interferência mantêm seus fluxos existentes.
