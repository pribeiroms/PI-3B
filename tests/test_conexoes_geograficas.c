#include <assert.h>
#include <stdio.h>

#include "grafo.h"

static void teste_conecta_antenas_elegiveis_mais_proximas(void)
{
    Grafo *grafo = grafo_criar();
    const NoAdjacencia *vizinhos;

    assert(grafo != NULL);
    assert(grafo_adicionar_antena(grafo, 724U, 5U, 10U, 1U,
           -23.5500, -46.6300, 700.0) == 0);
    assert(grafo_adicionar_antena(grafo, 724U, 5U, 10U, 2U,
           -23.5540, -46.6300, 100.0) == 1);
    assert(grafo_adicionar_antena(grafo, 724U, 5U, 10U, 3U,
           -23.5570, -46.6300, 100.0) == 2);

    assert(grafo_construir_conexoes(grafo) == 2U);
    assert(grafo_sao_adjacentes(grafo, 0U, 1U) == 1);
    assert(grafo_sao_adjacentes(grafo, 0U, 2U) == 1);
    assert(grafo_sao_adjacentes(grafo, 1U, 2U) == 0);

    vizinhos = grafo_vizinhos(grafo, 0U);
    assert(vizinhos != NULL && vizinhos->vertice == 2U);
    assert(vizinhos->proximo != NULL && vizinhos->proximo->vertice == 1U);
    assert(vizinhos->proximo->proximo == NULL);
    grafo_destruir(grafo);
}

static void teste_nao_conecta_antenas_fora_do_criterio(void)
{
    Grafo *grafo = grafo_criar();

    assert(grafo != NULL);
    assert(grafo_adicionar_antena(grafo, 724U, 5U, 10U, 1U,
           -23.5500, -46.6300, 1000.0) == 0);
    assert(grafo_adicionar_antena(grafo, 724U, 6U, 10U, 2U,
           -23.5501, -46.6300, 1000.0) == 1);
    assert(grafo_adicionar_antena(grafo, 724U, 5U, 11U, 3U,
           -23.5501, -46.6300, 1000.0) == 2);
    assert(grafo_adicionar_antena(grafo, 724U, 5U, 10U, 4U,
           -23.6500, -46.6300, 100.0) == 3);

    assert(grafo_construir_conexoes(grafo) == 0U);
    assert(grafo_quantidade_arestas(grafo) == 0U);
    grafo_destruir(grafo);
}

static void teste_construcao_e_reprodutivel(void)
{
    Grafo *grafo = grafo_criar();

    assert(grafo != NULL);
    assert(grafo_adicionar_antena(grafo, 724U, 5U, 10U, 1U,
           -23.5500, -46.6300, 1000.0) == 0);
    assert(grafo_adicionar_antena(grafo, 724U, 5U, 10U, 2U,
           -23.5540, -46.6300, 100.0) == 1);
    assert(grafo_adicionar_antena(grafo, 724U, 5U, 10U, 3U,
           -23.5570, -46.6300, 100.0) == 2);

    assert(grafo_construir_conexoes(grafo) == 2U);
    assert(grafo_sao_adjacentes(grafo, 0U, 1U) == 1);
    assert(grafo_sao_adjacentes(grafo, 0U, 2U) == 1);
    assert(grafo_construir_conexoes(grafo) == 2U);
    assert(grafo_sao_adjacentes(grafo, 0U, 1U) == 1);
    assert(grafo_sao_adjacentes(grafo, 0U, 2U) == 1);
    grafo_destruir(grafo);
}

int main(void)
{
    teste_conecta_antenas_elegiveis_mais_proximas();
    teste_nao_conecta_antenas_fora_do_criterio();
    teste_construcao_e_reprodutivel();
    puts("Testes de conexoes geograficas passaram.");
    return 0;
}
