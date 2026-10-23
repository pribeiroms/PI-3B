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

static void verificar_ordem(EstruturaGrafo estrutura, OrdemColoracao ordem,
    size_t vertices, const size_t (*arestas)[2], size_t quantidade_arestas,
    const size_t *cores_esperadas, size_t total_esperado)
{
    Grafo *grafo = montar(estrutura, vertices, arestas, quantidade_arestas);
    ResultadoColoracao resultado;
    size_t i;

    assert(coloracao_gulosa_ordem(grafo, ordem, &resultado) == 1);
    assert(resultado.quantidade_vertices == vertices);
    assert(resultado.total_cores == total_esperado);
    for (i = 0U; i < vertices; ++i) assert(resultado.cores[i] == cores_esperadas[i]);
    assert(coloracao_valida(grafo, &resultado) == 1);
    coloracao_liberar(&resultado);
    assert(resultado.cores == NULL && resultado.total_cores == 0U);
    grafo_destruir(grafo);
}

static void verificar(EstruturaGrafo estrutura, size_t vertices,
    const size_t (*arestas)[2], size_t quantidade_arestas,
    const size_t *cores_esperadas, size_t total_esperado)
{
    Grafo *grafo = montar(estrutura, vertices, arestas, quantidade_arestas);
    ResultadoColoracao resultado;
    size_t i;

    /* coloracao_gulosa continua equivalente a ordem natural. */
    assert(coloracao_gulosa(grafo, &resultado) == 1);
    assert(resultado.total_cores == total_esperado);
    for (i = 0U; i < vertices; ++i) assert(resultado.cores[i] == cores_esperadas[i]);
    coloracao_liberar(&resultado);
    grafo_destruir(grafo);
    verificar_ordem(estrutura, COLORACAO_ORDEM_NATURAL, vertices, arestas,
        quantidade_arestas, cores_esperadas, total_esperado);
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

/* Caminho 0-1-3-2-4 mais o vertice isolado 5: a ordem natural colore 0 e 2
 * primeiro, ambos com a cor 0, e forca o vertice 3 a usar uma terceira cor.
 * Welsh-Powell visita 1, 2, 3, 0, 4, 5 e resolve com duas cores.
 */
static void teste_welsh_powell(EstruturaGrafo estrutura)
{
    static const size_t arestas[][2] = {{0U, 1U}, {1U, 3U}, {2U, 3U}, {2U, 4U}};
    static const size_t graus_esperados[] = {1U, 2U, 2U, 2U, 1U, 0U};
    static const size_t ordem_esperada[] = {1U, 2U, 3U, 0U, 4U, 5U};
    static const size_t cores_natural[] = {0U, 1U, 0U, 2U, 1U, 0U};
    static const size_t cores_welsh_powell[] = {1U, 0U, 0U, 1U, 1U, 0U};
    static const size_t triangulo[][2] = {{0U, 1U}, {1U, 2U}, {0U, 2U}};
    static const size_t cores_triangulo[] = {0U, 1U, 2U};
    /* Estrela com centro no ultimo vertice: Welsh-Powell o visita primeiro. */
    static const size_t estrela[][2] = {{0U, 3U}, {1U, 3U}, {2U, 3U}};
    static const size_t cores_estrela[] = {1U, 1U, 1U, 0U};
    Grafo *grafo = montar(estrutura, 6U, arestas, 4U);
    size_t graus[6], ordem[6], i;
    ComparacaoColoracao comparacao;

    assert(coloracao_calcular_graus(grafo, graus) == 1);
    assert(coloracao_ordem_welsh_powell(grafo, ordem) == 1);
    for (i = 0U; i < 6U; ++i) {
        assert(graus[i] == graus_esperados[i]);
        assert(ordem[i] == ordem_esperada[i]);
    }
    assert(coloracao_comparar_ordens(grafo, &comparacao) == 1);
    assert(comparacao.cores_ordem_natural == 3U);
    assert(comparacao.cores_welsh_powell == 2U);
    grafo_destruir(grafo);

    verificar_ordem(estrutura, COLORACAO_ORDEM_NATURAL, 6U, arestas, 4U, cores_natural, 3U);
    verificar_ordem(estrutura, COLORACAO_WELSH_POWELL, 6U, arestas, 4U,
        cores_welsh_powell, 2U);
    verificar_ordem(estrutura, COLORACAO_WELSH_POWELL, 3U, triangulo, 3U,
        cores_triangulo, 3U);
    verificar_ordem(estrutura, COLORACAO_WELSH_POWELL, 4U, estrela, 3U, cores_estrela, 2U);
    verificar_ordem(estrutura, COLORACAO_WELSH_POWELL, 0U, NULL, 0U, NULL, 0U);
}

static void verificar_exata(EstruturaGrafo estrutura, size_t vertices,
    const size_t (*arestas)[2], size_t quantidade_arestas, size_t numero_cromatico)
{
    Grafo *grafo = montar(estrutura, vertices, arestas, quantidade_arestas);
    ResultadoColoracao resultado, gulosa;
    InfoColoracaoExata info;

    assert(coloracao_exata(grafo, NULL, &resultado, &info) == COLORACAO_EXATA_OK);
    assert(resultado.quantidade_vertices == vertices);
    assert(resultado.total_cores == numero_cromatico);
    assert(info.limite_inferior == numero_cromatico);
    assert(coloracao_valida(grafo, &resultado) == 1);
    /* A heuristica gulosa nunca usa menos cores que a solucao exata. */
    assert(coloracao_gulosa(grafo, &gulosa) == 1);
    assert(gulosa.total_cores >= resultado.total_cores);
    coloracao_liberar(&gulosa);
    coloracao_liberar(&resultado);
    grafo_destruir(grafo);
}

/* Grafo de Mycielski: sem triangulos e com numero cromatico crescente. A
 * partir de G (n vertices) cria copias u_i ligadas aos vizinhos de v_i e um
 * vertice w ligado a todas as copias. Comeca em K2 (cromatico 2).
 */
static Grafo *montar_mycielski(EstruturaGrafo estrutura, size_t nivel)
{
    size_t arestas[300][2], quantidade_arestas = 1U, n = 2U, i, a, b, atual;
    Grafo *grafo;

    arestas[0][0] = 0U;
    arestas[0][1] = 1U;
    for (; nivel > 2U; --nivel) {
        atual = quantidade_arestas;
        for (i = 0U; i < atual; ++i) {
            a = arestas[i][0];
            b = arestas[i][1];
            assert(quantidade_arestas + 2U <= 300U);
            arestas[quantidade_arestas][0] = n + a;
            arestas[quantidade_arestas++][1] = b;
            arestas[quantidade_arestas][0] = n + b;
            arestas[quantidade_arestas++][1] = a;
        }
        for (i = 0U; i < n; ++i) {
            assert(quantidade_arestas < 300U);
            arestas[quantidade_arestas][0] = n + i;
            arestas[quantidade_arestas++][1] = 2U * n;
        }
        n = 2U * n + 1U;
    }
    grafo = montar(estrutura, n, (const size_t (*)[2])arestas, quantidade_arestas);
    return grafo;
}

static void teste_coloracao_exata(EstruturaGrafo estrutura)
{
    static const size_t ciclo5[][2] = {{0U, 1U}, {1U, 2U}, {2U, 3U}, {3U, 4U}, {4U, 0U}};
    static const size_t caminho[][2] = {{0U, 1U}, {1U, 2U}, {2U, 3U}};
    static const size_t k4[][2] = {
        {0U, 1U}, {0U, 2U}, {0U, 3U}, {1U, 2U}, {1U, 3U}, {2U, 3U}
    };
    /* Coroa: a gulosa em ordem natural usa 3 cores, o otimo e 2. */
    static const size_t coroa[][2] = {
        {0U, 3U}, {1U, 2U}, {0U, 5U}, {4U, 1U}, {2U, 5U}, {3U, 4U}
    };
    static const size_t petersen[][2] = {
        {0U, 1U}, {1U, 2U}, {2U, 3U}, {3U, 4U}, {4U, 0U},
        {0U, 5U}, {1U, 6U}, {2U, 7U}, {3U, 8U}, {4U, 9U},
        {5U, 7U}, {7U, 9U}, {9U, 6U}, {6U, 8U}, {8U, 5U}
    };
    Grafo *grotzsch = montar_mycielski(estrutura, 4U);
    ResultadoColoracao resultado;
    InfoColoracaoExata info;

    verificar_exata(estrutura, 0U, NULL, 0U, 0U);
    verificar_exata(estrutura, 3U, NULL, 0U, 1U);
    verificar_exata(estrutura, 4U, caminho, 3U, 2U);
    verificar_exata(estrutura, 5U, ciclo5, 5U, 3U);
    verificar_exata(estrutura, 4U, k4, 6U, 4U);
    verificar_exata(estrutura, 6U, coroa, 6U, 2U);
    verificar_exata(estrutura, 10U, petersen, 15U, 3U);

    /* Grafo de Grotzsch (Mycielski de nivel 4): 11 vertices, cromatico 4. */
    assert(grafo_quantidade_vertices(grotzsch) == 11U);
    assert(coloracao_exata(grotzsch, NULL, &resultado, &info) == COLORACAO_EXATA_OK);
    assert(resultado.total_cores == 4U);
    assert(coloracao_valida(grotzsch, &resultado) == 1);
    coloracao_liberar(&resultado);
    grafo_destruir(grotzsch);
}

static void teste_limites_coloracao_exata(void)
{
    static const size_t triangulo[][2] = {{0U, 1U}, {1U, 2U}, {0U, 2U}};
    LimitesColoracaoExata so_tamanho = {2U, 0.0};
    LimitesColoracaoExata tamanho_suficiente = {3U, 0.0};
    LimitesColoracaoExata pouco_tempo = {0U, 0.05};
    Grafo *grafo = montar(GRAFO_LISTA_ADJACENCIA, 3U, triangulo, 3U);
    ResultadoColoracao resultado;
    InfoColoracaoExata info;

    assert(coloracao_exata(grafo, &so_tamanho, &resultado, &info) ==
        COLORACAO_EXATA_LIMITE_TAMANHO);
    assert(resultado.cores == NULL);
    assert(coloracao_exata(grafo, &tamanho_suficiente, &resultado, NULL) ==
        COLORACAO_EXATA_OK);
    assert(resultado.total_cores == 3U);
    coloracao_liberar(&resultado);
    grafo_destruir(grafo);

    /* Mycielski de nivel 6 (47 vertices, cromatico 6): provar que 5 cores nao
     * bastam leva muito mais que 50 ms, entao a busca e interrompida.
     */
    grafo = montar_mycielski(GRAFO_LISTA_ADJACENCIA, 6U);
    assert(grafo_quantidade_vertices(grafo) == 47U);
    assert(coloracao_exata(grafo, &pouco_tempo, &resultado, &info) ==
        COLORACAO_EXATA_LIMITE_TEMPO);
    assert(resultado.cores == NULL);
    assert(info.limite_inferior >= 2U && info.limite_inferior <= 6U);
    assert(info.nos_visitados > 0U);
    assert(info.segundos >= 0.05);
    grafo_destruir(grafo);

    assert(coloracao_exata(NULL, NULL, &resultado, NULL) == COLORACAO_EXATA_ERRO);
    assert(coloracao_mensagem_exata(COLORACAO_EXATA_LIMITE_TEMPO) != NULL);
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
    assert(coloracao_gulosa_ordem(grafo, (OrdemColoracao)99, &resultado) == 0);
    assert(coloracao_calcular_graus(NULL, NULL) == 0);
    assert(coloracao_ordem_welsh_powell(grafo, NULL) == 0);
    assert(coloracao_comparar_ordens(grafo, NULL) == 0);
    assert(coloracao_comparar_ordens(NULL, NULL) == 0);
    assert(coloracao_valida(grafo, &invalido) == 0);
    assert(coloracao_valida(NULL, &invalido) == 0);
    coloracao_liberar(NULL);
    grafo_destruir(grafo);
}

int main(void)
{
    size_t i;

    for (i = 0U; i < sizeof(ESTRUTURAS) / sizeof(ESTRUTURAS[0]); ++i) {
        teste_casos(ESTRUTURAS[i]);
        teste_welsh_powell(ESTRUTURAS[i]);
        teste_coloracao_exata(ESTRUTURAS[i]);
    }
    teste_limites_coloracao_exata();
    teste_entradas_invalidas();
    puts("Testes da coloracao passaram (gulosa, Welsh-Powell e exata).");
    return 0;
}
