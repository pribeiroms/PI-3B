#ifndef AGM_H
#define AGM_H

#include <stddef.h>

#include "grafo.h"

typedef struct {
	ArestaPonderada *arestas;
	size_t quantidade_arestas;
	size_t quantidade_arvores;
	double custo_total_km;
	size_t cruzamentos;
	size_t cruzamentos_grafo_candidato;
} ResultadoAgm;

/* Extrai e ordena arestas por peso crescente; retorna 1 em sucesso. */
int agm_extrair_ordenar_arestas(const Grafo *grafo,
								RepresentacaoGrafo representacao,
								ArestaPonderada **arestas, size_t *quantidade);

/* Executa Kruskal e retorna a AGM ou a floresta geradora minima. */
int agm_kruskal(const Grafo *grafo, RepresentacaoGrafo representacao,
				ResultadoAgm *resultado);

void agm_resultado_destruir(ResultadoAgm *resultado);

#endif
