#include <stdint.h>
#include <stdlib.h>

#include "coloracao.h"

#define SEM_COR SIZE_MAX

typedef struct {
    size_t vertice;
    size_t grau;
} VerticeGrau;

/* Marca as cores dos vizinhos ja coloridos de v. Em vez de limpar o vetor a
 * cada vertice, grava carimbo (passo + 1): so vale a marca do passo atual.
 */
static void marcar_cores_vizinhos(const Grafo *grafo, size_t v,
    const size_t *cores, size_t *marca, size_t carimbo)
{
    const NoAdjacencia *vizinho;
    size_t u, quantidade = grafo_quantidade_vertices(grafo);

    if (grafo_estrutura(grafo) == GRAFO_MATRIZ_ADJACENCIA) {
        for (u = 0U; u < quantidade; ++u)
            if (cores[u] != SEM_COR && grafo_sao_adjacentes(grafo, v, u))
                marca[cores[u]] = carimbo;
        return;
    }
    for (vizinho = grafo_vizinhos(grafo, v); vizinho != NULL; vizinho = vizinho->proximo)
        if (cores[vizinho->vertice] != SEM_COR) marca[cores[vizinho->vertice]] = carimbo;
}

static int comparar_grau_decrescente(const void *a, const void *b)
{
    const VerticeGrau *x = a, *y = b;
    if (x->grau != y->grau) return x->grau > y->grau ? -1 : 1;
    return x->vertice < y->vertice ? -1 : (x->vertice > y->vertice);
}

int coloracao_calcular_graus(const Grafo *grafo, size_t *graus)
{
    size_t indice, origem, destino;

    if (grafo == NULL || (graus == NULL && grafo_quantidade_vertices(grafo) > 0U)) return 0;
    for (indice = 0U; indice < grafo_quantidade_vertices(grafo); ++indice) graus[indice] = 0U;
    for (indice = 0U; indice < grafo_quantidade_arestas(grafo); ++indice) {
        if (!grafo_obter_aresta(grafo, indice, &origem, &destino)) return 0;
        ++graus[origem];
        ++graus[destino];
    }
    return 1;
}

int coloracao_ordem_welsh_powell(const Grafo *grafo, size_t *ordem)
{
    size_t quantidade, v;
    size_t *graus;
    VerticeGrau *pares;

    if (grafo == NULL) return 0;
    quantidade = grafo_quantidade_vertices(grafo);
    if (quantidade == 0U) return 1;
    if (ordem == NULL || quantidade > SIZE_MAX / sizeof(VerticeGrau)) return 0;

    graus = malloc(quantidade * sizeof(size_t));
    pares = malloc(quantidade * sizeof(VerticeGrau));
    if (graus == NULL || pares == NULL || !coloracao_calcular_graus(grafo, graus)) {
        free(graus);
        free(pares);
        return 0;
    }
    for (v = 0U; v < quantidade; ++v) {
        pares[v].vertice = v;
        pares[v].grau = graus[v];
    }
    qsort(pares, quantidade, sizeof(VerticeGrau), comparar_grau_decrescente);
    for (v = 0U; v < quantidade; ++v) ordem[v] = pares[v].vertice;
    free(graus);
    free(pares);
    return 1;
}

int coloracao_gulosa_ordem(const Grafo *grafo, OrdemColoracao ordem,
    ResultadoColoracao *resultado)
{
    size_t quantidade, passo, v, cor;
    size_t *marca, *sequencia;
    int ok;

    if (grafo == NULL || resultado == NULL ||
        (ordem != COLORACAO_ORDEM_NATURAL && ordem != COLORACAO_WELSH_POWELL)) return 0;
    resultado->cores = NULL;
    resultado->quantidade_vertices = 0U;
    resultado->total_cores = 0U;
    quantidade = grafo_quantidade_vertices(grafo);
    if (quantidade == 0U) return 1;
    if (quantidade > SIZE_MAX / sizeof(size_t)) return 0;

    resultado->cores = malloc(quantidade * sizeof(size_t));
    /* Um vertice nunca precisa de mais de V cores (0..V-1). */
    marca = calloc(quantidade, sizeof(size_t));
    sequencia = malloc(quantidade * sizeof(size_t));
    ok = resultado->cores != NULL && marca != NULL && sequencia != NULL;
    if (ok && ordem == COLORACAO_WELSH_POWELL)
        ok = coloracao_ordem_welsh_powell(grafo, sequencia);
    else if (ok)
        for (v = 0U; v < quantidade; ++v) sequencia[v] = v;
    if (!ok) {
        free(resultado->cores);
        free(marca);
        free(sequencia);
        resultado->cores = NULL;
        return 0;
    }

    for (v = 0U; v < quantidade; ++v) resultado->cores[v] = SEM_COR;
    for (passo = 0U; passo < quantidade; ++passo) {
        v = sequencia[passo];
        marcar_cores_vizinhos(grafo, v, resultado->cores, marca, passo + 1U);
        for (cor = 0U; marca[cor] == passo + 1U; ++cor) {}
        resultado->cores[v] = cor;
        if (cor + 1U > resultado->total_cores) resultado->total_cores = cor + 1U;
    }
    resultado->quantidade_vertices = quantidade;
    free(marca);
    free(sequencia);
    return 1;
}

int coloracao_gulosa(const Grafo *grafo, ResultadoColoracao *resultado)
{
    return coloracao_gulosa_ordem(grafo, COLORACAO_ORDEM_NATURAL, resultado);
}

int coloracao_comparar_ordens(const Grafo *grafo, ComparacaoColoracao *comparacao)
{
    ResultadoColoracao natural, welsh_powell;

    if (comparacao == NULL) return 0;
    if (!coloracao_gulosa_ordem(grafo, COLORACAO_ORDEM_NATURAL, &natural)) return 0;
    if (!coloracao_gulosa_ordem(grafo, COLORACAO_WELSH_POWELL, &welsh_powell)) {
        coloracao_liberar(&natural);
        return 0;
    }
    comparacao->cores_ordem_natural = natural.total_cores;
    comparacao->cores_welsh_powell = welsh_powell.total_cores;
    coloracao_liberar(&natural);
    coloracao_liberar(&welsh_powell);
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
