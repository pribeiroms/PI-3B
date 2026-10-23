#include <stdint.h>
#include <stdlib.h>

#include "coloracao.h"

/* Marca as cores dos vizinhos ja coloridos de v. Em vez de limpar o vetor a
 * cada vertice, grava v + 1 como carimbo: so vale a marca do vertice atual.
 */
static void marcar_cores_vizinhos(const Grafo *grafo, size_t v,
    const size_t *cores, size_t *marca)
{
    const NoAdjacencia *vizinho;
    size_t u;

    if (grafo_estrutura(grafo) == GRAFO_MATRIZ_ADJACENCIA) {
        for (u = 0U; u < v; ++u)
            if (grafo_sao_adjacentes(grafo, v, u)) marca[cores[u]] = v + 1U;
        return;
    }
    for (vizinho = grafo_vizinhos(grafo, v); vizinho != NULL; vizinho = vizinho->proximo)
        if (vizinho->vertice < v) marca[cores[vizinho->vertice]] = v + 1U;
}

int coloracao_gulosa(const Grafo *grafo, ResultadoColoracao *resultado)
{
    size_t quantidade, v, cor;
    size_t *marca;

    if (grafo == NULL || resultado == NULL) return 0;
    resultado->cores = NULL;
    resultado->quantidade_vertices = 0U;
    resultado->total_cores = 0U;
    quantidade = grafo_quantidade_vertices(grafo);
    if (quantidade == 0U) return 1;
    if (quantidade > SIZE_MAX / sizeof(size_t)) return 0;

    resultado->cores = malloc(quantidade * sizeof(size_t));
    /* Um vertice nunca precisa de mais de V cores (0..V-1). */
    marca = calloc(quantidade, sizeof(size_t));
    if (resultado->cores == NULL || marca == NULL) {
        free(resultado->cores);
        free(marca);
        resultado->cores = NULL;
        return 0;
    }

    for (v = 0U; v < quantidade; ++v) {
        marcar_cores_vizinhos(grafo, v, resultado->cores, marca);
        for (cor = 0U; marca[cor] == v + 1U; ++cor) {}
        resultado->cores[v] = cor;
        if (cor + 1U > resultado->total_cores) resultado->total_cores = cor + 1U;
    }
    resultado->quantidade_vertices = quantidade;
    free(marca);
    return 1;
}

int coloracao_valida(const Grafo *grafo, const ResultadoColoracao *resultado)
{
    size_t indice, origem, destino;

    if (grafo == NULL || resultado == NULL ||
        resultado->quantidade_vertices != grafo_quantidade_vertices(grafo)) return 0;
    for (indice = 0U; indice < grafo_quantidade_arestas(grafo); ++indice) {
        if (!grafo_obter_aresta(grafo, indice, &origem, &destino)) return 0;
        if (resultado->cores[origem] == resultado->cores[destino]) return 0;
    }
    return 1;
}

void coloracao_liberar(ResultadoColoracao *resultado)
{
    if (resultado == NULL) return;
    free(resultado->cores);
    resultado->cores = NULL;
    resultado->quantidade_vertices = 0U;
    resultado->total_cores = 0U;
}
