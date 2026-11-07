# Árvore Geradora Mínima (Fase II)

## Modelo e peso

Cada aresta candidata recebe o peso `peso_metros`. A inserção original
`grafo_adicionar_aresta` calcula esse valor pela distância de Haversine entre as
coordenadas dos vértices. A função adicional `grafo_adicionar_aresta_ponderada`
permite montar grafos pequenos com pesos conhecidos. O custo comunicado pelo
Kruskal é a soma dos pesos dividida por 1.000, em quilômetros.

## Estruturas e algoritmos

O módulo `conjuntos_disjuntos` inicializa cada vértice como conjunto unitário.
`find` aplica compressão de caminho; `union` usa rank. A extração percorre a
representação de Lista ou Matriz solicitada e copia cada par não direcionado uma
única vez. O vetor é ordenado crescentemente por peso com mergesort, em tempo
`O(E log E)` em todos os casos, usando um vetor auxiliar `O(E)`. O vetor de
arestas extraído também ocupa `O(E)` espaço.

Kruskal percorre as arestas ordenadas e escolhe uma aresta quando ela conecta
dois conjuntos diferentes. A estrutura disjunta usa `O(V)` espaço; as operações
de união/busca têm custo amortizado `O(α(V))`. O processamento após a ordenação
custa `O(E α(V) + V α(V))`. Se houver componentes desconexas, o resultado é uma
floresta geradora mínima; a contagem de raízes informa quantas árvores há,
incluindo vértices isolados. O vetor de saída é reservado com capacidade `O(E)`.

## Cruzamentos e interpretação

A rotina geométrica existente é reutilizada com os pares selecionados por
Kruskal, e os cruzamentos próprios entre segmentos sem vértice comum são
contados. O resultado também informa o total no grafo candidato para comparação.
Essa análise geométrica custa `O(E²)` no grafo candidato e `O(V²)` no máximo
para as arestas selecionadas.
Cada cruzamento sinaliza possível necessidade de isolamento, alteração do
traçado ou outra análise de engenharia. Coordenadas estimadas e segmentos em
plano não representam rotas executáveis de cabos; esse indicador não calcula
custo ou viabilidade de instalação.

## Decisões e interface

O grafo mantém a API original e recebe funções adicionais para pesos, extração e
contagem de cruzamentos. O módulo `agm` retorna as arestas escolhidas, quantidade
de árvores, custo em km e cruzamentos. O chamador libera a memória do resultado
com `agm_resultado_destruir`. Para adicionar arestas com custo de teste explícito,
use `grafo_adicionar_aresta_ponderada`. A suíte é executada por
`mingw32-make test`.
