#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "agm.h"

static Grafo *criar_grafo(EstruturaGrafo estrutura, size_t vertices)
{
    Grafo *g = grafo_criar_com_estrutura(estrutura);
    size_t i;
    assert(g != NULL);
    for (i = 0U; i < vertices; ++i)
        assert(grafo_adicionar_antena(g, 724U, 2U, 1U, i,
            0.0, (double)i, 10.0) == (int)i);
    return g;
}

static void adicionar(Grafo *g, size_t a, size_t b, double peso)
{
    assert(grafo_adicionar_aresta_ponderada(g, a, b, peso));
}

static void teste_union_find(void)
{
    ConjuntosDisjuntos *ds = conjuntos_disjuntos_criar(6U);
    assert(ds != NULL);
    assert(conjuntos_disjuntos_quantidade(ds) == 6U);
    assert(conjuntos_disjuntos_find(ds, 3U) == 3U);
    assert(conjuntos_disjuntos_union(ds, 0U, 1U));
    assert(conjuntos_disjuntos_union(ds, 1U, 2U));
    assert(conjuntos_disjuntos_find(ds, 0U) ==
        conjuntos_disjuntos_find(ds, 2U));
    assert(!conjuntos_disjuntos_union(ds, 0U, 2U));
    assert(conjuntos_disjuntos_find(ds, 6U) == (size_t)-1);
    conjuntos_disjuntos_destruir(ds);
    conjuntos_disjuntos_destruir(NULL);
}

static Grafo *criar_grafo_conhecido(EstruturaGrafo estrutura)
{
    Grafo *g = grafo_criar_com_estrutura(estrutura);
    assert(g != NULL);
    assert(grafo_adicionar_antena(g, 724U, 2U, 1U, 0U, 0.0, 0.0, 10.0) == 0);
    assert(grafo_adicionar_antena(g, 724U, 2U, 1U, 1U, 1.0, 1.0, 10.0) == 1);
    assert(grafo_adicionar_antena(g, 724U, 2U, 1U, 2U, 1.0, 0.0, 10.0) == 2);
    assert(grafo_adicionar_antena(g, 724U, 2U, 1U, 3U, 0.0, 1.0, 10.0) == 3);
    /* Quadrado: as duas diagonais cruzam, mas sao mais caras que o contorno. */
    adicionar(g, 0U, 1U, 9.0);
    adicionar(g, 2U, 3U, 8.0);
    adicionar(g, 0U, 2U, 1.0);
    adicionar(g, 2U, 1U, 2.0);
    adicionar(g, 1U, 3U, 3.0);
    adicionar(g, 3U, 0U, 4.0);
    return g;
}

static void testar_estrutura(EstruturaGrafo estrutura)
{
    Grafo *g = criar_grafo_conhecido(estrutura), *floresta = NULL;
    RelatorioAGM r;
    Aresta *ordenadas = NULL;
    size_t n = 0U, i;
    const size_t origens_esperadas[] = {0U, 2U, 1U};
    const size_t destinos_esperados[] = {2U, 1U, 3U};
    Cruzamento *cruzamentos = NULL;
    size_t cruzamentos_candidato, cruzamentos_agm;

    assert(agm_extrair_arestas_ordenadas(g, &ordenadas, &n));
    assert(n == 6U);
    for (i = 1U; i < n; ++i) assert(ordenadas[i - 1U].peso <= ordenadas[i].peso);
    assert(ordenadas[0].origem == 0U && ordenadas[0].destino == 2U);
    free(ordenadas);

    assert(agm_kruskal(g, &floresta, &r));
    assert(r.vertices == 4U && r.arestas == 3U && r.arvores == 1U);
    assert(fabs(r.custo_total_km - 6.0) < 1e-9);
    assert(grafo_quantidade_arestas(floresta) == 3U);
    for (i = 0U; i < 3U; ++i) {
        size_t origem, destino;
        assert(grafo_obter_aresta_ponderada(floresta, i, &origem, &destino, NULL));
        assert(origem == origens_esperadas[i]);
        assert(destino == destinos_esperados[i]);
    }
    cruzamentos_candidato = grafo_detectar_cruzamentos(g, NULL);
    cruzamentos_agm = grafo_detectar_cruzamentos(floresta, &cruzamentos);
    assert(cruzamentos_candidato == 1U);
    assert(cruzamentos_agm == 0U);
    free(cruzamentos);
    grafo_destruir(floresta);
    grafo_destruir(g);
}

static void teste_floresta_desconexa(void)
{
    Grafo *g = criar_grafo(GRAFO_LISTA_ADJACENCIA, 6U), *floresta = NULL;
    RelatorioAGM r;
    adicionar(g, 0U, 1U, 1.0);
    adicionar(g, 1U, 2U, 2.0);
    adicionar(g, 0U, 2U, 5.0);
    adicionar(g, 3U, 4U, 4.0);
    assert(agm_kruskal(g, &floresta, &r));
    assert(r.vertices == 6U && r.arestas == 3U && r.arvores == 3U);
    assert(fabs(r.custo_total_km - 7.0) < 1e-9);
    grafo_destruir(floresta);
    grafo_destruir(g);
}

int main(void)
{
    teste_union_find();
    testar_estrutura(GRAFO_LISTA_ADJACENCIA);
    testar_estrutura(GRAFO_MATRIZ_ADJACENCIA);
    teste_floresta_desconexa();
    puts("Testes da AGM passaram.");
    return 0;
}
