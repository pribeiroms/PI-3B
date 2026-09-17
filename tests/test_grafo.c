#include <assert.h>
#include <stdio.h>

#include "grafo.h"

static void teste_cria_e_destroi_grafo(void)
{
    Grafo *grafo = grafo_criar();

    assert(grafo != NULL);
    grafo_destruir(grafo);
}

static void teste_cria_conexao_elegivel(void)
{
    Grafo *grafo = grafo_criar();
    assert(grafo != NULL);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, 1U, -23.0, -46.0, 1000.0) == 0);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, 2U, -23.0, -46.005, 1000.0) == 1);
    assert(grafo_adicionar_antena(grafo, 724U, 3U, 10U, 3U, -23.0, -46.004, 1000.0) == 2);
    assert(grafo_construir_conexoes(grafo) == 1U);
    assert(grafo_quantidade_arestas(grafo) == 1U);
    grafo_destruir(grafo);
}

static void teste_detecta_cruzamento(void)
{
    Grafo *grafo = grafo_criar();
    assert(grafo != NULL);
    assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 1U, 0.0, 0.0, 1.0) == 0);
    assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 2U, 1.0, 1.0, 1.0) == 1);
    assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 3U, 0.0, 1.0, 1.0) == 2);
    assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 4U, 1.0, 0.0, 1.0) == 3);
    assert(grafo_adicionar_aresta(grafo, 0U, 1U));
    assert(grafo_adicionar_aresta(grafo, 2U, 3U));
    assert(grafo_possui_cruzamentos(grafo));
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
    teste_cria_conexao_elegivel();
    teste_detecta_cruzamento();
    teste_representacoes_do_grafo();
    puts("Todos os testes passaram.");
    return 0;
}
