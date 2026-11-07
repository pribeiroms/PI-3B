#include "agm.h"

#include <stdlib.h>

#include "conjuntos_disjuntos.h"

static void trocar(ArestaPonderada *a, ArestaPonderada *b)
{
	ArestaPonderada temporaria = *a;
	*a = *b;
	*b = temporaria;
}

static void ordenar(ArestaPonderada *arestas, size_t inicio, size_t fim)
{
	size_t esquerda = inicio, direita = fim;
	double pivo;
	if (fim - inicio < 2U) return;
	pivo = arestas[inicio + (fim - inicio) / 2U].peso_metros;
	while (esquerda < direita) {
		while (arestas[esquerda].peso_metros < pivo) ++esquerda;
		while (arestas[direita - 1U].peso_metros > pivo) --direita;
		if (esquerda < direita) {
			trocar(&arestas[esquerda], &arestas[direita - 1U]);
			++esquerda;
			--direita;
		}
	}
	if (inicio < direita) ordenar(arestas, inicio, direita);
	if (esquerda < fim) ordenar(arestas, esquerda, fim);
}

int agm_extrair_ordenar_arestas(const Grafo *grafo,
								RepresentacaoGrafo representacao,
								ArestaPonderada **arestas, size_t *quantidade)
{
	size_t total;
	if (arestas == NULL || quantidade == NULL) return 0;
	*arestas = NULL;
	*quantidade = 0U;
	total = grafo_extrair_arestas_ponderadas(grafo, representacao, arestas);
	if (grafo == NULL) return 0;
	if (grafo_quantidade_arestas(grafo) > 0U && total == 0U) return 0;
	*quantidade = total;
	if (total > 1U) ordenar(*arestas, 0U, total);
	return 1;
}

int agm_kruskal(const Grafo *grafo, RepresentacaoGrafo representacao,
				ResultadoAgm *resultado)
{
	ArestaPonderada *ordenadas = NULL;
	ArestaPonderada *selecionadas = NULL;
	ConjuntosDisjuntos *conjuntos;
	size_t quantidade, vertices, selecionadas_n = 0U, indice;
	ResultadoAgm calculado = {NULL, 0U, 0U, 0.0, 0U, 0U};
	if (grafo == NULL || resultado == NULL) return 0;
	*resultado = calculado;
	vertices = grafo_quantidade_vertices(grafo);
	if (!agm_extrair_ordenar_arestas(grafo, representacao, &ordenadas, &quantidade)) return 0;
	if (quantidade > 0U) {
		selecionadas = malloc(quantidade * sizeof(*selecionadas));
		if (selecionadas == NULL) { free(ordenadas); return 0; }
	}
	conjuntos = conjuntos_disjuntos_criar(vertices);
	if (conjuntos == NULL) { free(ordenadas); free(selecionadas); return 0; }
	for (indice = 0U; indice < quantidade; ++indice) {
		int unido = conjuntos_disjuntos_union(conjuntos, ordenadas[indice].origem,
												 ordenadas[indice].destino);
		if (unido < 0) {
			conjuntos_disjuntos_destruir(conjuntos); free(ordenadas); free(selecionadas);
			return 0;
		}
		if (unido == 1) {
			selecionadas[selecionadas_n++] = ordenadas[indice];
			calculado.custo_total_km += ordenadas[indice].peso_metros / 1000.0;
		}
	}
	for (indice = 0U; indice < vertices; ++indice)
		if (conjuntos_disjuntos_find(conjuntos, indice) == indice)
			++calculado.quantidade_arvores;
	conjuntos_disjuntos_destruir(conjuntos);
	free(ordenadas);
	if (selecionadas_n == 0U) { free(selecionadas); selecionadas = NULL; }
	else {
		ArestaPonderada *compactadas = realloc(selecionadas,
													 selecionadas_n * sizeof(*selecionadas));
		if (compactadas != NULL) selecionadas = compactadas;
	}
	calculado.arestas = selecionadas;
	calculado.quantidade_arestas = selecionadas_n;
	{
		Aresta *pares = NULL;
		if (selecionadas_n > 0U) {
			pares = malloc(selecionadas_n * sizeof(*pares));
			if (pares == NULL) { free(selecionadas); return 0; }
			for (indice = 0U; indice < selecionadas_n; ++indice) {
				pares[indice].origem = selecionadas[indice].origem;
				pares[indice].destino = selecionadas[indice].destino;
			}
		}
		calculado.cruzamentos = grafo_contar_cruzamentos_arestas(grafo, pares, selecionadas_n);
		free(pares);
	}
	calculado.cruzamentos_grafo_candidato = grafo_contar_cruzamentos_arestas(
		grafo, (const Aresta *)NULL, 0U);
	/* Obtencao dos pares candidatos pela matriz evita expor estado interno do Grafo. */
	{
		ArestaPonderada *candidatas = NULL;
		size_t n = grafo_extrair_arestas_ponderadas(grafo, representacao, &candidatas);
		Aresta *pares = n == 0U ? NULL : malloc(n * sizeof(*pares));
		if (n > 0U && pares != NULL) {
			for (indice = 0U; indice < n; ++indice) {
				pares[indice].origem = candidatas[indice].origem;
				pares[indice].destino = candidatas[indice].destino;
			}
			calculado.cruzamentos_grafo_candidato = grafo_contar_cruzamentos_arestas(grafo, pares, n);
		}
		free(pares); free(candidatas);
	}
	*resultado = calculado;
	return 1;
}

void agm_resultado_destruir(ResultadoAgm *resultado)
{
	if (resultado == NULL) return;
	free(resultado->arestas);
	*resultado = (ResultadoAgm){NULL, 0U, 0U, 0.0, 0U, 0U};
}
