#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "agm.h"
#include "conjuntos_disjuntos.h"

static Grafo *criar_grafo_pequeno(void)
{
	Grafo *grafo = grafo_criar();
	assert(grafo != NULL);
	assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 1U, 0.0, 0.0, 0.0) == 0);
	assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 2U, 0.0, 1.0, 0.0) == 1);
	assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 3U, 1.0, 0.0, 0.0) == 2);
	assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 4U, 5.0, 5.0, 0.0) == 3);
	assert(grafo_adicionar_aresta_ponderada(grafo, 0U, 1U, 1000.0) == 1);
	assert(grafo_adicionar_aresta_ponderada(grafo, 1U, 2U, 2000.0) == 1);
	assert(grafo_adicionar_aresta_ponderada(grafo, 0U, 2U, 5000.0) == 1);
	return grafo;
}

int main(void)
{
	ConjuntosDisjuntos *dsu = conjuntos_disjuntos_criar(3U);
	Grafo *grafo = criar_grafo_pequeno();
	ResultadoAgm lista, matriz;
	ArestaPonderada *ordenadas = NULL;
	size_t quantidade = 0U;
	assert(dsu != NULL);
	assert(conjuntos_disjuntos_find(dsu, 0U) != conjuntos_disjuntos_find(dsu, 1U));
	assert(conjuntos_disjuntos_union(dsu, 0U, 1U) == 1);
	assert(conjuntos_disjuntos_union(dsu, 1U, 0U) == 0);
	assert(conjuntos_disjuntos_find(dsu, 0U) == conjuntos_disjuntos_find(dsu, 1U));
	conjuntos_disjuntos_destruir(dsu);

	assert(agm_extrair_ordenar_arestas(grafo, GRAFO_LISTA_ADJACENCIA,
										 &ordenadas, &quantidade) == 1);
	assert(quantidade == 3U);
	assert(ordenadas[0].peso_metros == 1000.0 && ordenadas[2].peso_metros == 5000.0);
	free(ordenadas);
	assert(agm_kruskal(grafo, GRAFO_LISTA_ADJACENCIA, &lista) == 1);
	assert(agm_kruskal(grafo, GRAFO_MATRIZ_ADJACENCIA, &matriz) == 1);
	assert(lista.quantidade_arestas == 2U);
	assert(lista.quantidade_arvores == 2U);
	assert(fabs(lista.custo_total_km - 3.0) < 1e-9);
	assert(lista.quantidade_arestas == matriz.quantidade_arestas);
	assert(fabs(lista.custo_total_km - matriz.custo_total_km) < 1e-9);
	assert(lista.cruzamentos == matriz.cruzamentos);
	agm_resultado_destruir(&lista);
	agm_resultado_destruir(&matriz);
	grafo_destruir(grafo);

	grafo = grafo_criar();
	assert(grafo != NULL);
	assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 1U, 0.0, 0.0, 0.0) == 0);
	assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 2U, 0.0, 1.0, 0.0) == 1);
	assert(agm_kruskal(grafo, GRAFO_LISTA_ADJACENCIA, &lista) == 1);
	assert(lista.quantidade_arvores == 2U && lista.quantidade_arestas == 0U);
	agm_resultado_destruir(&lista);
	grafo_destruir(grafo);
	puts("Testes da AGM passaram.");
	return 0;
}
