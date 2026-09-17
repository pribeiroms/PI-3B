#ifndef GRAFO_H
#define GRAFO_H

/*
 * Interface inicial do módulo de grafos.
 * A estrutura e os algoritmos devem ser implementados autoralmente.
 */

typedef struct Grafo Grafo;

/* Cria um grafo vazio. Retorna NULL caso a alocação falhe. */
Grafo *grafo_criar(void);

/* Libera todos os recursos associados ao grafo. */
void grafo_destruir(Grafo *grafo);

#endif
