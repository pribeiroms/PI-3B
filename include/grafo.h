#ifndef GRAFO_H
#define GRAFO_H

#include <stddef.h>

typedef struct Grafo Grafo;

typedef struct {
	double latitude;
	double longitude;
} CoordenadaGeografica;

typedef struct {
	size_t id;
	unsigned int mcc;
	unsigned int net;
	unsigned int area;
	unsigned int cell;
	CoordenadaGeografica coordenadas;
	double alcance_metros;
} Vertice;

typedef struct {
	size_t origem;
	size_t destino;
} Aresta;

typedef struct NoAdjacencia NoAdjacencia;

struct NoAdjacencia {
	size_t vertice;
	NoAdjacencia *proximo;
};

typedef struct {
	NoAdjacencia **listas;
	size_t quantidade_vertices;
} ListaAdjacencia;

typedef struct {
	unsigned char *dados;
	size_t ordem;
} MatrizAdjacencia;

/* Cria um grafo vazio. Retorna NULL caso a alocação falhe. */
Grafo *grafo_criar(void);

/* Libera todos os recursos associados ao grafo. */
void grafo_destruir(Grafo *grafo);

int grafo_adicionar_vertice(Grafo *grafo, Vertice vertice);
int grafo_adicionar_aresta(Grafo *grafo, size_t origem, size_t destino);
size_t grafo_quantidade_vertices(const Grafo *grafo);
size_t grafo_quantidade_arestas(const Grafo *grafo);
int grafo_sao_adjacentes(const Grafo *grafo, size_t origem, size_t destino);

#endif
