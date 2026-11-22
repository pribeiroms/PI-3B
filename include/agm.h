#ifndef AGM_H
#define AGM_H

#include <stddef.h>

#include "grafo.h"

typedef struct ConjuntosDisjuntos ConjuntosDisjuntos;

/* Union-Find com compressao de caminho e uniao por rank. */
ConjuntosDisjuntos *conjuntos_disjuntos_criar(size_t quantidade);
void conjuntos_disjuntos_destruir(ConjuntosDisjuntos *conjuntos);
/* Retorna SIZE_MAX quando conjuntos/e elemento forem invalidos. */
size_t conjuntos_disjuntos_find(ConjuntosDisjuntos *conjuntos, size_t elemento);
/* Retorna 1 quando une dois conjuntos distintos; caso contrario, 0. */
int conjuntos_disjuntos_union(ConjuntosDisjuntos *conjuntos, size_t a, size_t b);
size_t conjuntos_disjuntos_quantidade(const ConjuntosDisjuntos *conjuntos);

typedef struct {
    size_t vertices;
    size_t arestas;
    size_t arvores;
    double custo_total_km;
} RelatorioAGM;

/* Copia e ordena em O(E log E) as arestas do grafo. */
int agm_extrair_arestas_ordenadas(const Grafo *grafo, Aresta **arestas,
    size_t *quantidade);

/* Devolve uma floresta nova com os mesmos vertices e apenas as arestas
 * escolhidas por Kruskal. O chamador passa a ser dono do grafo retornado. */
int agm_kruskal(const Grafo *grafo, Grafo **floresta, RelatorioAGM *relatorio);

#endif
