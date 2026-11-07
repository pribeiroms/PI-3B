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

static void teste_acessa_vizinhos(void)
{
    Grafo *grafo = grafo_criar();
    const NoAdjacencia *vizinhos;

    assert(grafo != NULL);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, 1U, -23.0, -46.0, 1000.0) == 0);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, 2U, -23.0, -46.005, 1000.0) == 1);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, 3U, -23.0, -46.006, 1000.0) == 2);
    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_adicionar_aresta(grafo, 0U, 2U) == 1);

    vizinhos = grafo_vizinhos(grafo, 0U);
    assert(vizinhos != NULL);
    assert(vizinhos->vertice == 2U);
    assert(vizinhos->proximo != NULL);
    assert(vizinhos->proximo->vertice == 1U);
    assert(vizinhos->proximo->proximo == NULL);

    vizinhos = grafo_vizinhos(grafo, 1U);
    assert(vizinhos != NULL);
    assert(vizinhos->vertice == 0U);
    assert(vizinhos->proximo == NULL);

    grafo_destruir(grafo);
}

static void teste_nao_adjacentes_e_vizinhos_vazios(void)
{
    Grafo *grafo = grafo_criar();

    assert(grafo != NULL);
    assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 1U, 0.0, 0.0, 1.0) == 0);
    assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 2U, 5.0, 5.0, 1.0) == 1);

    assert(grafo_sao_adjacentes(grafo, 0U, 1U) == 0);
    assert(grafo_vizinhos(grafo, 0U) == NULL);
    assert(grafo_vizinhos(grafo, 1U) == NULL);

    grafo_destruir(grafo);
}

static void teste_suporta_mil_vertices(void)
{
    Grafo *grafo = grafo_criar();
    size_t i;

    assert(grafo != NULL);
    for (i = 0U; i < 1000U; ++i) {
        double latitude = -23.0 + (double)i * 0.0001;
        assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, (unsigned int)i,
            latitude, -46.0, 500.0) == (int)i);
    }
    assert(grafo_quantidade_vertices(grafo) == 1000U);
    (void)grafo_construir_conexoes(grafo);
    grafo_destruir(grafo);
}

static Grafo *criar_grafo_com_vertices(size_t quantidade)
{
    Grafo *grafo = grafo_criar();
    size_t i;

    assert(grafo != NULL);
    for (i = 0U; i < quantidade; ++i) {
        assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, (unsigned int)i,
                                      0.0, (double)i * 0.001, 1.0) == (int)i);
    }
    return grafo;
}

static void teste_euler_nao_aplicavel(void)
{
    Grafo *grafo = criar_grafo_com_vertices(2U);

    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_verificar_euler(grafo) == EULER_NAO_APLICAVEL);
    assert(grafo_verificar_euler(NULL) == EULER_NAO_APLICAVEL);
    grafo_destruir(grafo);
}

static void teste_euler_triangulo_inconclusivo(void)
{
    Grafo *grafo = criar_grafo_com_vertices(3U);

    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_adicionar_aresta(grafo, 1U, 2U) == 1);
    assert(grafo_adicionar_aresta(grafo, 0U, 2U) == 1);
    assert(grafo_verificar_euler(grafo) == EULER_INCONCLUSIVO);
    grafo_destruir(grafo);
}

static void teste_euler_nao_planar(void)
{
    Grafo *grafo = criar_grafo_com_vertices(5U);
    size_t i, j;

    for (i = 0U; i < 5U; ++i)
        for (j = i + 1U; j < 5U; ++j)
            assert(grafo_adicionar_aresta(grafo, i, j) == 1);

    assert(grafo_quantidade_arestas(grafo) == 10U);
    assert(grafo_verificar_euler(grafo) == EULER_NAO_PLANAR);
    grafo_destruir(grafo);
}

static void teste_euler_inconclusivo(void)
{
    Grafo *grafo = criar_grafo_com_vertices(6U);
    size_t i, j;

    for (i = 0U; i < 3U; ++i)
        for (j = 3U; j < 6U; ++j)
            assert(grafo_adicionar_aresta(grafo, i, j) == 1);

    assert(grafo_quantidade_arestas(grafo) == 9U);
    assert(grafo_verificar_euler(grafo) == EULER_INCONCLUSIVO);
    grafo_destruir(grafo);
}

static void teste_quantidades_apos_construcao_automatica(void)
{
    Grafo *grafo = grafo_criar();
    const size_t vertices_esperados = 5U;
    size_t i, j;

    assert(grafo != NULL);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, 1U, -23.000, -46.000, 600.0) == 0);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, 2U, -23.001, -46.000, 600.0) == 1);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, 3U, -23.002, -46.000, 600.0) == 2);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, 4U, -23.100, -46.100, 600.0) == 3);
    assert(grafo_adicionar_antena(grafo, 999U, 9U, 90U, 5U, 10.000, 10.000, 600.0) == 4);

    assert(grafo_quantidade_vertices(grafo) == vertices_esperados);
    assert(grafo_quantidade_arestas(grafo) == 0U);

    (void)grafo_construir_conexoes(grafo);

    assert(grafo_quantidade_vertices(grafo) == vertices_esperados);
    for (i = 0U; i < vertices_esperados; ++i)
        for (j = i + 1U; j < vertices_esperados; ++j)
            if (grafo_sao_adjacentes(grafo, i, j))
                assert(grafo_sao_adjacentes(grafo, j, i));

    grafo_destruir(grafo);
}

static void teste_obtem_segmento_da_aresta(void)
{
    Grafo *grafo = grafo_criar();
    Segmento segmento;

    assert(grafo != NULL);
    assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 1U, -23.0, -46.0, 1.0) == 0);
    assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, 2U, -22.5, -45.5, 1.0) == 1);
    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);

    assert(grafo_obter_segmento(grafo, 0U, &segmento) == 1);
    assert(segmento.inicio.latitude == -23.0);
    assert(segmento.inicio.longitude == -46.0);
    assert(segmento.fim.latitude == -22.5);
    assert(segmento.fim.longitude == -45.5);

    /* entradas invalidas */
    assert(grafo_obter_segmento(grafo, 1U, &segmento) == 0);
    assert(grafo_obter_segmento(grafo, 0U, NULL) == 0);
    assert(grafo_obter_segmento(NULL, 0U, &segmento) == 0);
    grafo_destruir(grafo);
}

static void teste_arestas_compartilham_vertice(void)
{
    Grafo *grafo = criar_grafo_com_vertices(4U);

    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_adicionar_aresta(grafo, 1U, 2U) == 1);
    assert(grafo_adicionar_aresta(grafo, 2U, 3U) == 1);

    assert(grafo_arestas_compartilham_vertice(grafo, 0U, 1U) == 1);
    assert(grafo_arestas_compartilham_vertice(grafo, 1U, 2U) == 1);
    assert(grafo_arestas_compartilham_vertice(grafo, 0U, 2U) == 0);
    assert(grafo_arestas_compartilham_vertice(grafo, 0U, 9U) == 0);
    assert(grafo_arestas_compartilham_vertice(NULL, 0U, 1U) == 0);
    grafo_destruir(grafo);
}

int main(void)
{
    teste_cria_e_destroi_grafo();
    teste_cria_conexao_elegivel();
    teste_detecta_cruzamento();
    teste_representacoes_do_grafo();
    teste_acessa_vizinhos();
    teste_nao_adjacentes_e_vizinhos_vazios();
    teste_suporta_mil_vertices();
    teste_euler_nao_aplicavel();
    teste_euler_triangulo_inconclusivo();
    teste_euler_nao_planar();
    teste_euler_inconclusivo();
    teste_quantidades_apos_construcao_automatica();
    teste_obtem_segmento_da_aresta();
    teste_arestas_compartilham_vertice();
    puts("Todos os testes passaram.");
    return 0;
}