#ifndef CONJUNTOS_DISJUNTOS_H
#define CONJUNTOS_DISJUNTOS_H

#include <stddef.h>
#include <stdint.h>

typedef struct ConjuntosDisjuntos ConjuntosDisjuntos;

ConjuntosDisjuntos *conjuntos_disjuntos_criar(size_t quantidade_elementos);
size_t conjuntos_disjuntos_find(ConjuntosDisjuntos *conjuntos, size_t elemento);
int conjuntos_disjuntos_union(ConjuntosDisjuntos *conjuntos, size_t primeiro,
							  size_t segundo);
void conjuntos_disjuntos_destruir(ConjuntosDisjuntos *conjuntos);

#endif
