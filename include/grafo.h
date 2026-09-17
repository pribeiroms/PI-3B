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

/* Adiciona um vertice e retorna seu indice, ou -1 caso a entrada seja invalida. */
int grafo_adicionar_vertice(Grafo *grafo, Vertice vertice);

/* Adiciona uma antena georreferenciada e retorna seu indice, ou -1 em erro. */
int grafo_adicionar_antena(Grafo *grafo, unsigned int mcc, unsigned int net,
						   unsigned int area, unsigned int cell, double latitude,
						   double longitude, double alcance_metros);

/* Adiciona uma aresta nao direcionada e sem peso. Retorna 1 em caso de sucesso. */
int grafo_adicionar_aresta(Grafo *grafo, size_t origem, size_t destino);

/* Le antenas de um CSV OpenCelliD; max_antenas igual a 0 le todos os registros. */
size_t grafo_carregar_csv(Grafo *grafo, const char *caminho, size_t max_antenas,
						  char *erro, size_t tamanho_erro);

/* Cria as arestas da Fase I conforme os criterios de proximidade e alcance. */
size_t grafo_construir_conexoes(Grafo *grafo);

size_t grafo_quantidade_vertices(const Grafo *grafo);
size_t grafo_quantidade_arestas(const Grafo *grafo);

/* Retorna 1 se existir cruzamento entre duas arestas sem vertice em comum. */
int grafo_possui_cruzamentos(const Grafo *grafo);

int grafo_sao_adjacentes(const Grafo *grafo, size_t origem, size_t destino);

#endif
