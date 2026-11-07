#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "agm.h"

struct ConjuntosDisjuntos {
    size_t *pai;
    unsigned char *rank;
    size_t quantidade;
};

ConjuntosDisjuntos *conjuntos_disjuntos_criar(size_t quantidade)
{
    ConjuntosDisjuntos *c = calloc(1U, sizeof(*c));
    size_t i;
    if (c == NULL) return NULL;
    c->quantidade = quantidade;
    if (quantidade == 0U) return c;
    if (quantidade > SIZE_MAX / sizeof(*c->pai)) { free(c); return NULL; }
    c->pai = malloc(quantidade * sizeof(*c->pai));
    c->rank = calloc(quantidade, sizeof(*c->rank));
    if (c->pai == NULL || c->rank == NULL) {
        conjuntos_disjuntos_destruir(c);
        return NULL;
    }
    for (i = 0U; i < quantidade; ++i) c->pai[i] = i;
    return c;
}

void conjuntos_disjuntos_destruir(ConjuntosDisjuntos *c)
{
    if (c == NULL) return;
    free(c->pai);
    free(c->rank);
    free(c);
}

size_t conjuntos_disjuntos_find(ConjuntosDisjuntos *c, size_t elemento)
{
    size_t raiz, atual;
    if (c == NULL || elemento >= c->quantidade) return SIZE_MAX;
    raiz = elemento;
    while (c->pai[raiz] != raiz) raiz = c->pai[raiz];
    atual = elemento;
    while (c->pai[atual] != atual) {
        size_t proximo = c->pai[atual];
        c->pai[atual] = raiz;
        atual = proximo;
    }
    return raiz;
}

int conjuntos_disjuntos_union(ConjuntosDisjuntos *c, size_t a, size_t b)
{
    size_t ra = conjuntos_disjuntos_find(c, a);
    size_t rb = conjuntos_disjuntos_find(c, b);
    if (ra == SIZE_MAX || rb == SIZE_MAX || ra == rb) return 0;
    if (c->rank[ra] < c->rank[rb]) c->pai[ra] = rb;
    else if (c->rank[ra] > c->rank[rb]) c->pai[rb] = ra;
    else { c->pai[rb] = ra; ++c->rank[ra]; }
    return 1;
}

size_t conjuntos_disjuntos_quantidade(const ConjuntosDisjuntos *c)
{
    return c == NULL ? 0U : c->quantidade;
}

static int comparar_arestas(const void *pa, const void *pb)
{
    const Aresta *a = pa, *b = pb;
    if (a->peso < b->peso) return -1;
    if (a->peso > b->peso) return 1;
    if (a->origem < b->origem) return -1;
    if (a->origem > b->origem) return 1;
    if (a->destino < b->destino) return -1;
    if (a->destino > b->destino) return 1;
    return 0;
}

int agm_extrair_arestas_ordenadas(const Grafo *grafo, Aresta **arestas,
                                  size_t *quantidade)
{
    Aresta *copia = NULL;
    size_t n, i;
    if (arestas == NULL || quantidade == NULL || grafo == NULL) return 0;
    *arestas = NULL;
    *quantidade = 0U;
    n = grafo_quantidade_arestas(grafo);
    if (n > SIZE_MAX / sizeof(*copia)) return 0;
    if (n != 0U) {
        copia = malloc(n * sizeof(*copia));
        if (copia == NULL) return 0;
    }
    for (i = 0U; i < n; ++i) {
        if (!grafo_obter_aresta_ponderada(grafo, i, &copia[i].origem,
                &copia[i].destino, &copia[i].peso)) {
            free(copia);
            return 0;
        }
    }
    if (n > 1U) qsort(copia, n, sizeof(*copia), comparar_arestas);
    *arestas = copia;
    *quantidade = n;
    return 1;
}

int agm_kruskal(const Grafo *grafo, Grafo **floresta, RelatorioAGM *relatorio)
{
    Aresta *arestas = NULL;
    size_t n_arestas = 0U, vertices, i;
    ConjuntosDisjuntos *ds = NULL;
    Grafo *resultado = NULL;
    RelatorioAGM r = {0U, 0U, 0U, 0.0};
    if (grafo == NULL || floresta == NULL || relatorio == NULL) return 0;
    *floresta = NULL;
    memset(relatorio, 0, sizeof(*relatorio));
    vertices = grafo_quantidade_vertices(grafo);
    if (!agm_extrair_arestas_ordenadas(grafo, &arestas, &n_arestas)) return 0;
    ds = conjuntos_disjuntos_criar(vertices);
    resultado = grafo_criar_com_estrutura(grafo_estrutura(grafo));
    if (ds == NULL || resultado == NULL) goto erro;
    for (i = 0U; i < vertices; ++i) {
        Vertice v;
        if (!grafo_obter_vertice(grafo, i, &v) ||
            grafo_adicionar_vertice(resultado, v) < 0) goto erro;
    }
    for (i = 0U; i < n_arestas; ++i) {
        if (!conjuntos_disjuntos_union(ds, arestas[i].origem, arestas[i].destino))
            continue;
        if (!grafo_adicionar_aresta_ponderada(resultado, arestas[i].origem,
                arestas[i].destino, arestas[i].peso)) goto erro;
        r.custo_total_km += arestas[i].peso;
        if (!isfinite(r.custo_total_km)) goto erro;
        ++r.arestas;
    }
    r.vertices = vertices;
    /* Conta raizes: a quantidade alocada e V, nao o numero de componentes. */
    r.arvores = 0U;
    for (i = 0U; i < vertices; ++i)
        if (conjuntos_disjuntos_find(ds, i) == i) ++r.arvores;
    free(arestas);
    conjuntos_disjuntos_destruir(ds);
    *floresta = resultado;
    *relatorio = r;
    return 1;
erro:
    free(arestas);
    conjuntos_disjuntos_destruir(ds);
    grafo_destruir(resultado);
    return 0;
}
