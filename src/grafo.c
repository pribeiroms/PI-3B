#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "grafo.h"

struct Grafo {
    Vertice *vertices;
    size_t quantidade_vertices;
    size_t capacidade_vertices;
    Aresta *arestas;
    size_t quantidade_arestas;
    size_t capacidade_arestas;
    ListaAdjacencia lista_adjacencia;
    MatrizAdjacencia matriz_adjacencia;
};

static int reservar_vertices(Grafo *grafo)
{
    size_t capacidade = grafo->capacidade_vertices == 0U ? 8U : grafo->capacidade_vertices * 2U;
    Vertice *vertices = realloc(grafo->vertices, capacidade * sizeof(*vertices));

    if (vertices == NULL) return 0;
    grafo->vertices = vertices;
    grafo->capacidade_vertices = capacidade;
    return 1;
}

static int reservar_arestas(Grafo *grafo)
{
    size_t capacidade = grafo->capacidade_arestas == 0U ? 8U : grafo->capacidade_arestas * 2U;
    Aresta *arestas = realloc(grafo->arestas, capacidade * sizeof(*arestas));

    if (arestas == NULL) return 0;
    grafo->arestas = arestas;
    grafo->capacidade_arestas = capacidade;
    return 1;
}

static int expandir_adjacencia(Grafo *grafo)
{
    size_t antiga_ordem = grafo->matriz_adjacencia.ordem;
    size_t nova_ordem = antiga_ordem == 0U ? 8U : antiga_ordem * 2U;
    NoAdjacencia **listas = realloc(grafo->lista_adjacencia.listas,
                                   nova_ordem * sizeof(*listas));
    unsigned char *dados;
    size_t linha;

    if (listas == NULL) return 0;
    dados = calloc(nova_ordem * nova_ordem, sizeof(*dados));
    if (dados == NULL) {
        grafo->lista_adjacencia.listas = listas;
        return 0;
    }
    for (linha = 0U; linha < antiga_ordem; ++linha) {
        memcpy(&dados[linha * nova_ordem],
               &grafo->matriz_adjacencia.dados[linha * antiga_ordem],
               antiga_ordem * sizeof(*dados));
    }
    for (linha = antiga_ordem; linha < nova_ordem; ++linha) listas[linha] = NULL;
    free(grafo->matriz_adjacencia.dados);
    grafo->lista_adjacencia.listas = listas;
    grafo->lista_adjacencia.quantidade_vertices = nova_ordem;
    grafo->matriz_adjacencia.dados = dados;
    grafo->matriz_adjacencia.ordem = nova_ordem;
    return 1;
}

static void liberar_lista(NoAdjacencia *lista)
{
    while (lista != NULL) {
        NoAdjacencia *proximo = lista->proximo;
        free(lista);
        lista = proximo;
    }
}

Grafo *grafo_criar(void)
{
    return calloc(1, sizeof(Grafo));
}

void grafo_destruir(Grafo *grafo)
{
    size_t indice;

    if (grafo == NULL) return;
    for (indice = 0U; indice < grafo->lista_adjacencia.quantidade_vertices; ++indice)
        liberar_lista(grafo->lista_adjacencia.listas[indice]);
    free(grafo->lista_adjacencia.listas);
    free(grafo->matriz_adjacencia.dados);
    free(grafo->vertices);
    free(grafo->arestas);
    free(grafo);
}

int grafo_adicionar_vertice(Grafo *grafo, Vertice vertice)
{
    if (grafo == NULL || !isfinite(vertice.coordenadas.latitude) ||
        !isfinite(vertice.coordenadas.longitude) || !isfinite(vertice.alcance_metros) ||
        vertice.coordenadas.latitude < -90.0 || vertice.coordenadas.latitude > 90.0 ||
        vertice.coordenadas.longitude < -180.0 || vertice.coordenadas.longitude > 180.0 ||
        vertice.alcance_metros < 0.0) return -1;
    if (grafo->quantidade_vertices == grafo->capacidade_vertices && !reservar_vertices(grafo))
        return -1;
    if (grafo->quantidade_vertices == grafo->matriz_adjacencia.ordem && !expandir_adjacencia(grafo))
        return -1;
    vertice.id = grafo->quantidade_vertices;
    grafo->vertices[grafo->quantidade_vertices] = vertice;
    ++grafo->quantidade_vertices;
    return (int)vertice.id;
}

int grafo_adicionar_aresta(Grafo *grafo, size_t origem, size_t destino)
{
    NoAdjacencia *origem_no;
    NoAdjacencia *destino_no;

    if (grafo == NULL || origem == destino || origem >= grafo->quantidade_vertices ||
        destino >= grafo->quantidade_vertices || grafo_sao_adjacentes(grafo, origem, destino))
        return 0;
    if (grafo->quantidade_arestas == grafo->capacidade_arestas && !reservar_arestas(grafo))
        return 0;
    origem_no = malloc(sizeof(*origem_no));
    destino_no = malloc(sizeof(*destino_no));
    if (origem_no == NULL || destino_no == NULL) {
        free(origem_no);
        free(destino_no);
        return 0;
    }
    origem_no->vertice = destino;
    origem_no->proximo = grafo->lista_adjacencia.listas[origem];
    destino_no->vertice = origem;
    destino_no->proximo = grafo->lista_adjacencia.listas[destino];
    grafo->lista_adjacencia.listas[origem] = origem_no;
    grafo->lista_adjacencia.listas[destino] = destino_no;
    grafo->matriz_adjacencia.dados[origem * grafo->matriz_adjacencia.ordem + destino] = 1U;
    grafo->matriz_adjacencia.dados[destino * grafo->matriz_adjacencia.ordem + origem] = 1U;
    grafo->arestas[grafo->quantidade_arestas].origem = origem;
    grafo->arestas[grafo->quantidade_arestas].destino = destino;
    ++grafo->quantidade_arestas;
    return 1;
}

size_t grafo_quantidade_vertices(const Grafo *grafo)
{
    return grafo == NULL ? 0U : grafo->quantidade_vertices;
}

size_t grafo_quantidade_arestas(const Grafo *grafo)
{
    return grafo == NULL ? 0U : grafo->quantidade_arestas;
}

int grafo_sao_adjacentes(const Grafo *grafo, size_t origem, size_t destino)
{
    if (grafo == NULL || origem >= grafo->matriz_adjacencia.ordem ||
        destino >= grafo->matriz_adjacencia.ordem) return 0;
    return grafo->matriz_adjacencia.dados[origem * grafo->matriz_adjacencia.ordem + destino] != 0U;
}
