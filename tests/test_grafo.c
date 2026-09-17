#include <assert.h>
#include <stdio.h>

#include "grafo.h"

static void teste_cria_e_destroi_grafo(void)
{
    Grafo *grafo = grafo_criar();

    assert(grafo != NULL);
    grafo_destruir(grafo);
}

static void teste_representacoes_do_grafo(void)
{
    Grafo *grafo = grafo_criar();
    Vertice origem = {0U, 724U, 10U, 20U, 1U, {-23.55, -46.63}, 1000.0};
    Vertice destino = {0U, 724U, 10U, 20U, 2U, {-23.56, -46.64}, 1000.0};

    assert(grafo != NULL);
    assert(grafo_adicionar_vertice(grafo, origem) == 0);
    assert(grafo_adicionar_vertice(grafo, destino) == 1);
    assert(grafo_quantidade_vertices(grafo) == 2U);
    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_quantidade_arestas(grafo) == 1U);
    assert(grafo_sao_adjacentes(grafo, 0U, 1U) == 1);
    assert(grafo_sao_adjacentes(grafo, 1U, 0U) == 1);
    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 0);
    grafo_destruir(grafo);
}

int main(void)
{
    teste_cria_e_destroi_grafo();
    teste_representacoes_do_grafo();
    puts("Todos os testes passaram.");
    return 0;
}
