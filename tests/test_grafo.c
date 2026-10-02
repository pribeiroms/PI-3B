#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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

static Grafo *criar_grafo_com_coordenadas(const double latitudes[], const double longitudes[],
    size_t quantidade)
{
    Grafo *grafo = grafo_criar();
    size_t i;

    assert(grafo != NULL);
    for (i = 0U; i < quantidade; ++i)
        assert(grafo_adicionar_antena(grafo, 1U, 1U, 1U, (unsigned int)i,
            latitudes[i], longitudes[i], 1.0) == (int)i);
    return grafo;
}

static void teste_planaridade_arvore(void)
{
    double lat[] = {0.0, 1.0, 0.0, -1.0};
    double lon[] = {0.0, 0.0, 1.0, 0.0};
    Grafo *grafo = criar_grafo_com_coordenadas(lat, lon, 4U);
    Cruzamento *cruzamentos;
    size_t quantidade;

    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_adicionar_aresta(grafo, 0U, 2U) == 1);
    assert(grafo_adicionar_aresta(grafo, 0U, 3U) == 1);

    assert(grafo_verificar_euler(grafo) == EULER_INCONCLUSIVO);
    quantidade = grafo_detectar_cruzamentos(grafo, &cruzamentos);
    assert(quantidade == 0U);
    free(cruzamentos);

    grafo_destruir(grafo);
}

static void teste_planaridade_ciclo(void)
{
    double lat[] = {0.0, 0.0, 2.0, 3.0, 1.0};
    double lon[] = {0.0, 2.0, 3.0, 1.0, -1.0};
    Grafo *grafo = criar_grafo_com_coordenadas(lat, lon, 5U);
    Cruzamento *cruzamentos;
    size_t quantidade;

    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_adicionar_aresta(grafo, 1U, 2U) == 1);
    assert(grafo_adicionar_aresta(grafo, 2U, 3U) == 1);
    assert(grafo_adicionar_aresta(grafo, 3U, 4U) == 1);
    assert(grafo_adicionar_aresta(grafo, 4U, 0U) == 1);

    assert(grafo_verificar_euler(grafo) == EULER_INCONCLUSIVO);
    quantidade = grafo_detectar_cruzamentos(grafo, &cruzamentos);
    assert(quantidade == 0U);
    free(cruzamentos);

    grafo_destruir(grafo);
}

static void teste_planaridade_k4_planar(void)
{
    /* Triangulo (0,1,2) com o vertice 3 no centro: desenho planar classico de K4. */
    double lat[] = {0.0, 0.0, 4.0, 1.5};
    double lon[] = {0.0, 4.0, 2.0, 2.0};
    Grafo *grafo = criar_grafo_com_coordenadas(lat, lon, 4U);
    Cruzamento *cruzamentos;
    size_t quantidade;

    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_adicionar_aresta(grafo, 1U, 2U) == 1);
    assert(grafo_adicionar_aresta(grafo, 2U, 0U) == 1);
    assert(grafo_adicionar_aresta(grafo, 0U, 3U) == 1);
    assert(grafo_adicionar_aresta(grafo, 1U, 3U) == 1);
    assert(grafo_adicionar_aresta(grafo, 2U, 3U) == 1);

    assert(grafo_quantidade_arestas(grafo) == 6U);
    assert(grafo_verificar_euler(grafo) == EULER_INCONCLUSIVO);
    quantidade = grafo_detectar_cruzamentos(grafo, &cruzamentos);
    assert(quantidade == 0U);
    free(cruzamentos);

    grafo_destruir(grafo);
}

static void teste_planaridade_k5_nao_planar(void)
{
    /* Mesmos 5 pontos do ciclo, mas com TODAS as conexoes (K5): nunca da pra
     * desenhar sem cruzar, nao importa o layout. Aqui o Euler ja acerta sozinho. */
    double lat[] = {0.0, 0.0, 2.0, 3.0, 1.0};
    double lon[] = {0.0, 2.0, 3.0, 1.0, -1.0};
    Grafo *grafo = criar_grafo_com_coordenadas(lat, lon, 5U);
    Cruzamento *cruzamentos;
    size_t quantidade, i, j;

    for (i = 0U; i < 5U; ++i)
        for (j = i + 1U; j < 5U; ++j)
            assert(grafo_adicionar_aresta(grafo, i, j) == 1);

    assert(grafo_quantidade_arestas(grafo) == 10U);
    assert(grafo_verificar_euler(grafo) == EULER_NAO_PLANAR);
    quantidade = grafo_detectar_cruzamentos(grafo, &cruzamentos);
    assert(quantidade > 0U);
    free(cruzamentos);

    grafo_destruir(grafo);
}

static void teste_planaridade_k3_3_limitacao_euler(void)
{
    /* K3,3: 3 antenas "em cima", 3 "embaixo", todas interligadas.
     * Euler (E <= 3V-6) fica satisfeito (9 <= 12) e diz INCONCLUSIVO,
     * como se pudesse ser planar -- mas K3,3 NUNCA e planar, e o
     * desenho real tem cruzamento. Isso demonstra a limitacao de usar
     * só Euler, exigida pela issue #21. */
    double lat[] = {2.0, 2.0, 2.0, 0.0, 0.0, 0.0};
    double lon[] = {0.0, 2.0, 4.0, 0.0, 2.0, 4.0};
    Grafo *grafo = criar_grafo_com_coordenadas(lat, lon, 6U);
    Cruzamento *cruzamentos;
    size_t quantidade, i, j;

    for (i = 0U; i < 3U; ++i)
        for (j = 3U; j < 6U; ++j)
            assert(grafo_adicionar_aresta(grafo, i, j) == 1);

    assert(grafo_quantidade_arestas(grafo) == 9U);
    assert(grafo_verificar_euler(grafo) == EULER_INCONCLUSIVO);
    quantidade = grafo_detectar_cruzamentos(grafo, &cruzamentos);
    assert(quantidade > 0U);
    free(cruzamentos);

    grafo_destruir(grafo);
}

static void teste_desenho_sem_cruzamentos(void)
{
    /* Quadrado, so os lados (sem diagonais): nenhum cruzamento. */
    double lat[] = {0.0, 0.0, 2.0, 2.0};
    double lon[] = {0.0, 2.0, 2.0, 0.0};
    Grafo *grafo = criar_grafo_com_coordenadas(lat, lon, 4U);
    Cruzamento *cruzamentos;
    size_t quantidade;

    assert(grafo_adicionar_aresta(grafo, 0U, 1U) == 1);
    assert(grafo_adicionar_aresta(grafo, 1U, 2U) == 1);
    assert(grafo_adicionar_aresta(grafo, 2U, 3U) == 1);
    assert(grafo_adicionar_aresta(grafo, 3U, 0U) == 1);

    quantidade = grafo_detectar_cruzamentos(grafo, &cruzamentos);
    assert(quantidade == 0U);
    free(cruzamentos);

    grafo_destruir(grafo);
}

static void teste_desenho_com_cruzamentos_explicitos(void)
{
    /* Mesmo quadrado, mas conectando as duas DIAGONAIS: cruzam no centro. */
    double lat[] = {0.0, 0.0, 2.0, 2.0};
    double lon[] = {0.0, 2.0, 2.0, 0.0};
    Grafo *grafo = criar_grafo_com_coordenadas(lat, lon, 4U);
    Cruzamento *cruzamentos;
    size_t quantidade;

    assert(grafo_adicionar_aresta(grafo, 0U, 2U) == 1);
    assert(grafo_adicionar_aresta(grafo, 1U, 3U) == 1);

    quantidade = grafo_detectar_cruzamentos(grafo, &cruzamentos);
    assert(quantidade == 1U);
    assert(cruzamentos[0].aresta_a == 0U);
    assert(cruzamentos[0].aresta_b == 1U);
    free(cruzamentos);

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
    teste_planaridade_arvore();
    teste_planaridade_ciclo();
    teste_planaridade_k4_planar();
    teste_planaridade_k5_nao_planar();
    teste_planaridade_k3_3_limitacao_euler();
    teste_desenho_sem_cruzamentos();
    teste_desenho_com_cruzamentos_explicitos();
    puts("Todos os testes passaram.");
    return 0;
}