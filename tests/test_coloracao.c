#include <assert.h>
#include <stdio.h>

#include "coloracao.h"

static const EstruturaGrafo ESTRUTURAS[] = {
    GRAFO_LISTA_ADJACENCIA, GRAFO_MATRIZ_ADJACENCIA, GRAFO_ESTRUTURA_CONJUNTA
};

/* Monta um grafo a mao, sem coordenadas relevantes. */
static Grafo *montar(EstruturaGrafo estrutura, size_t vertices,
    const size_t (*arestas)[2], size_t quantidade_arestas)
{
    Grafo *grafo = grafo_criar_com_estrutura(estrutura);
    Vertice vertice = {0U, 724U, 5U, 10U, 0U, {-23.5505, -46.6333}, 900.0};
    size_t i;

    assert(grafo != NULL);
    for (i = 0U; i < vertices; ++i) {
        vertice.cell = (unsigned int)i;
        assert(grafo_adicionar_vertice(grafo, vertice) == (int)i);
    }
    for (i = 0U; i < quantidade_arestas; ++i)
        assert(grafo_adicionar_aresta(grafo, arestas[i][0], arestas[i][1]) == 1);
    return grafo;
}

static void verificar(EstruturaGrafo estrutura, size_t vertices,
    const size_t (*arestas)[2], size_t quantidade_arestas,
    const size_t *cores_esperadas, size_t total_esperado)
{
    Grafo *grafo = montar(estrutura, vertices, arestas, quantidade_arestas);
    ResultadoColoracao resultado;
    size_t i;

    assert(coloracao_gulosa(grafo, &resultado) == 1);
    assert(resultado.quantidade_vertices == vertices);
    assert(resultado.total_cores == total_esperado);
    for (i = 0U; i < vertices; ++i) assert(resultado.cores[i] == cores_esperadas[i]);
    assert(coloracao_valida(grafo, &resultado) == 1);
    coloracao_liberar(&resultado);
    assert(resultado.cores == NULL && resultado.total_cores == 0U);
    grafo_destruir(grafo);
}

static void teste_casos(EstruturaGrafo estrutura)
{
    static const size_t triangulo[][2] = {{0U, 1U}, {1U, 2U}, {0U, 2U}};
    static const size_t cores_triangulo[] = {0U, 1U, 2U};
    static const size_t caminho[][2] = {{0U, 1U}, {1U, 2U}, {2U, 3U}};
    static const size_t cores_caminho[] = {0U, 1U, 0U, 1U};
    static const size_t cores_isolados[] = {0U, 0U, 0U};
    static const size_t k4[][2] = {
        {0U, 1U}, {0U, 2U}, {0U, 3U}, {1U, 2U}, {1U, 3U}, {2U, 3U}
    };
    static const size_t cores_k4[] = {0U, 1U, 2U, 3U};
    /* Coroa de 6 vertices: bipartido, mas a ordem natural forca 3 cores. */
    static const size_t coroa[][2] = {
        {0U, 3U}, {1U, 2U}, {0U, 5U}, {4U, 1U}, {2U, 5U}, {3U, 4U}
    };
    static const size_t cores_coroa[] = {0U, 0U, 1U, 1U, 2U, 2U};

    verificar(estrutura, 0U, NULL, 0U, NULL, 0U);
    verificar(estrutura, 1U, NULL, 0U, cores_isolados, 1U);
    verificar(estrutura, 3U, NULL, 0U, cores_isolados, 1U);
    verificar(estrutura, 3U, triangulo, 3U, cores_triangulo, 3U);
    verificar(estrutura, 4U, caminho, 3U, cores_caminho, 2U);
    verificar(estrutura, 4U, k4, 6U, cores_k4, 4U);
    verificar(estrutura, 6U, coroa, 6U, cores_coroa, 3U);
}

static void teste_entradas_invalidas(void)
{
    static const size_t aresta[][2] = {{0U, 1U}};
    Grafo *grafo = montar(GRAFO_LISTA_ADJACENCIA, 2U, aresta, 1U);
    ResultadoColoracao resultado;
    size_t cores_iguais[] = {0U, 0U};
    ResultadoColoracao invalido = {cores_iguais, 2U, 1U};

    assert(coloracao_gulosa(NULL, &resultado) == 0);
    assert(coloracao_gulosa(grafo, NULL) == 0);
    assert(coloracao_valida(grafo, &invalido) == 0);
    assert(coloracao_valida(NULL, &invalido) == 0);
    coloracao_liberar(NULL);
    grafo_destruir(grafo);
}

int main(void)
{
    size_t i;

    for (i = 0U; i < sizeof(ESTRUTURAS) / sizeof(ESTRUTURAS[0]); ++i)
        teste_casos(ESTRUTURAS[i]);
    teste_entradas_invalidas();
    puts("Testes da coloracao gulosa passaram.");
    return 0;
}
