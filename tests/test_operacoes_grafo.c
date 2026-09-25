#include <assert.h>
#include <stdio.h>

#include "grafo.h"

static Vertice criar_vertice(unsigned int cell)
{
    Vertice vertice = {0U, 724U, 5U, 10U, cell, {-23.5505, -46.6333}, 900.0};
    return vertice;
}

static void teste_criacao_do_grafo(void)
{
    Grafo *grafo = grafo_criar();

    assert(grafo != NULL);
    assert(grafo_quantidade_vertices(grafo) == 0U);
    assert(grafo_quantidade_arestas(grafo) == 0U);
    grafo_destruir(grafo);
}

static void teste_insercao_de_vertices(void)
{
    Grafo *grafo = grafo_criar();

    assert(grafo != NULL);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(1U)) == 0);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(2U)) == 1);
    assert(grafo_quantidade_vertices(grafo) == 2U);
    grafo_destruir(grafo);
}

static void teste_insercao_de_arestas(void)
{
    Grafo *grafo = grafo_criar();

    assert(grafo != NULL);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(1U)) == 0);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(2U)) == 1);
    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 0);
    assert(grafo_adicionar_aresta(grafo, 0U, 0U) == 0);
    assert(grafo_adicionar_aresta(grafo, 0U, 2U) == 0);
    grafo_destruir(grafo);
}

static void teste_consulta_de_adjacencia(void)
{
    Grafo *grafo = grafo_criar();

    assert(grafo != NULL);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(1U)) == 0);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(2U)) == 1);
    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_sao_adjacentes(grafo, 0U, 1U) == 1);
    assert(grafo_sao_adjacentes(grafo, 1U, 0U) == 1);
    assert(grafo_sao_adjacentes(grafo, 0U, 2U) == 0);
    grafo_destruir(grafo);
}

static void teste_contagem_de_vertices_e_arestas(void)
{
    Grafo *grafo = grafo_criar();

    assert(grafo != NULL);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(1U)) == 0);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(2U)) == 1);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(3U)) == 2);
    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_adicionar_aresta(grafo, 1U, 2U) == 1);
    assert(grafo_quantidade_vertices(grafo) == 3U);
    assert(grafo_quantidade_arestas(grafo) == 2U);
    grafo_destruir(grafo);
}

static void teste_lista_de_adjacencia(void)
{
    Grafo *grafo = grafo_criar();
    const NoAdjacencia *vizinhos;

    assert(grafo != NULL);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(1U)) == 0);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(2U)) == 1);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(3U)) == 2);
    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_adicionar_aresta(grafo, 0U, 2U) == 1);
    vizinhos = grafo_vizinhos(grafo, 0U);
    assert(vizinhos != NULL && vizinhos->vertice == 2U);
    assert(vizinhos->proximo != NULL && vizinhos->proximo->vertice == 1U);
    assert(vizinhos->proximo->proximo == NULL);
    grafo_destruir(grafo);
}

static void teste_liberacao_de_memoria(void)
{
    Grafo *grafo = grafo_criar();

    assert(grafo != NULL);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(1U)) == 0);
    assert(grafo_adicionar_vertice(grafo, criar_vertice(2U)) == 1);
    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    grafo_destruir(grafo);
    grafo_destruir(NULL);
}

int main(void)
{
    teste_criacao_do_grafo();
    teste_insercao_de_vertices();
    teste_insercao_de_arestas();
    teste_consulta_de_adjacencia();
    teste_contagem_de_vertices_e_arestas();
    teste_lista_de_adjacencia();
    teste_liberacao_de_memoria();
    puts("Testes das operacoes fundamentais passaram.");
    return 0;
}
