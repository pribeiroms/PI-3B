#include "conjuntos_disjuntos.h"

#include <stdlib.h>

struct ConjuntosDisjuntos {
	size_t quantidade;
	size_t *pais;
	size_t *ranks;
};

ConjuntosDisjuntos *conjuntos_disjuntos_criar(size_t quantidade_elementos)
{
	ConjuntosDisjuntos *conjuntos = malloc(sizeof(*conjuntos));
	size_t indice;
	if (conjuntos == NULL) return NULL;
	conjuntos->quantidade = quantidade_elementos;
	conjuntos->pais = NULL;
	conjuntos->ranks = NULL;
	if (quantidade_elementos == 0U) return conjuntos;
	if (quantidade_elementos > SIZE_MAX / sizeof(*conjuntos->pais) ||
		quantidade_elementos > SIZE_MAX / sizeof(*conjuntos->ranks)) {
		free(conjuntos);
		return NULL;
	}
	conjuntos->pais = malloc(quantidade_elementos * sizeof(*conjuntos->pais));
	conjuntos->ranks = calloc(quantidade_elementos, sizeof(*conjuntos->ranks));
	if (conjuntos->pais == NULL || conjuntos->ranks == NULL) {
		conjuntos_disjuntos_destruir(conjuntos);
		return NULL;
	}
	for (indice = 0U; indice < quantidade_elementos; ++indice)
		conjuntos->pais[indice] = indice;
	return conjuntos;
}

size_t conjuntos_disjuntos_find(ConjuntosDisjuntos *conjuntos, size_t elemento)
{
	if (conjuntos == NULL || elemento >= conjuntos->quantidade) return SIZE_MAX;
	if (conjuntos->pais[elemento] != elemento)
		conjuntos->pais[elemento] = conjuntos_disjuntos_find(
			conjuntos, conjuntos->pais[elemento]);
	return conjuntos->pais[elemento];
}

int conjuntos_disjuntos_union(ConjuntosDisjuntos *conjuntos, size_t primeiro,
							  size_t segundo)
{
	size_t raiz_primeiro = conjuntos_disjuntos_find(conjuntos, primeiro);
	size_t raiz_segundo = conjuntos_disjuntos_find(conjuntos, segundo);
	if (raiz_primeiro == SIZE_MAX || raiz_segundo == SIZE_MAX) return -1;
	if (raiz_primeiro == raiz_segundo) return 0;
	if (conjuntos->ranks[raiz_primeiro] < conjuntos->ranks[raiz_segundo])
		conjuntos->pais[raiz_primeiro] = raiz_segundo;
	else if (conjuntos->ranks[raiz_primeiro] > conjuntos->ranks[raiz_segundo])
		conjuntos->pais[raiz_segundo] = raiz_primeiro;
	else {
		conjuntos->pais[raiz_segundo] = raiz_primeiro;
		++conjuntos->ranks[raiz_primeiro];
	}
	return 1;
}

void conjuntos_disjuntos_destruir(ConjuntosDisjuntos *conjuntos)
{
	if (conjuntos == NULL) return;
	free(conjuntos->pais);
	free(conjuntos->ranks);
	free(conjuntos);
}
