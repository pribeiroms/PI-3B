#include <assert.h>
#include <stdio.h>

#include "coloracao.h"

/* Testes das propriedades da coloracao gulosa e da exata (#83). Cada familia
 * de grafos tem numero cromatico conhecido; os grafos aleatorios conferem que
 * a gulosa e sempre valida e nunca usa menos cores que a exata.
 */

static const EstruturaGrafo ESTRUTURAS[] = {
    GRAFO_LISTA_ADJACENCIA, GRAFO_MATRIZ_ADJACENCIA, GRAFO_ESTRUTURA_CONJUNTA
};
#define QUANTIDADE_ESTRUTURAS (sizeof(ESTRUTURAS) / sizeof(ESTRUTURAS[0]))

static Grafo *criar_com_vertices(EstruturaGrafo estrutura, size_t vertices)
{
    Grafo *grafo = grafo_criar_com_estrutura(estrutura);
    Vertice vertice = {0U, 724U, 5U, 10U, 0U, {-23.5505, -46.6333}, 900.0};
    size_t i;

    assert(grafo != NULL);
    for (i = 0U; i < vertices; ++i) {
        vertice.cell = (unsigned int)i;
        assert(grafo_adicionar_vertice(grafo, vertice) == (int)i);
    }
    return grafo;
}

static Grafo *montar_completo(EstruturaGrafo estrutura, size_t n)
{
    Grafo *grafo = criar_com_vertices(estrutura, n);
    size_t a, b;

    for (a = 0U; a < n; ++a)
        for (b = a + 1U; b < n; ++b) assert(grafo_adicionar_aresta(grafo, a, b) == 1);
    return grafo;
}

static Grafo *montar_ciclo(EstruturaGrafo estrutura, size_t n)
{
    Grafo *grafo = criar_com_vertices(estrutura, n);
    size_t i;

    for (i = 0U; i < n; ++i) assert(grafo_adicionar_aresta(grafo, i, (i + 1U) % n) == 1);
    return grafo;
}

/* Bipartido completo K(m,n): lados 0..m-1 e m..m+n-1. */
static Grafo *montar_bipartido_completo(EstruturaGrafo estrutura, size_t m, size_t n)
{
    Grafo *grafo = criar_com_vertices(estrutura, m + n);
    size_t a, b;

    for (a = 0U; a < m; ++a)
        for (b = m; b < m + n; ++b) assert(grafo_adicionar_aresta(grafo, a, b) == 1);
    return grafo;
}

/* Coroa com 2n vertices: K(n,n) sem o emparelhamento perfeito, com os lados
 * intercalados (pares de um lado, impares do outro). Bipartido, mas a gulosa
 * em ordem natural chega a usar n cores.
 */
static Grafo *montar_coroa(EstruturaGrafo estrutura, size_t n)
{
    Grafo *grafo = criar_com_vertices(estrutura, 2U * n);
    size_t a, b;

    for (a = 0U; a < n; ++a)
        for (b = 0U; b < n; ++b)
            if (a != b) assert(grafo_adicionar_aresta(grafo, 2U * a, 2U * b + 1U) == 1);
    return grafo;
}

/* Grade linhas x colunas: bipartida pela paridade de linha + coluna. */
static Grafo *montar_grade(EstruturaGrafo estrutura, size_t linhas, size_t colunas)
{
    Grafo *grafo = criar_com_vertices(estrutura, linhas * colunas);
    size_t l, c, v;

    for (l = 0U; l < linhas; ++l) {
        for (c = 0U; c < colunas; ++c) {
            v = l * colunas + c;
            if (c + 1U < colunas) assert(grafo_adicionar_aresta(grafo, v, v + 1U) == 1);
            if (l + 1U < linhas) assert(grafo_adicionar_aresta(grafo, v, v + colunas) == 1);
        }
    }
    return grafo;
}

/* Gerador congruente linear: os grafos "aleatorios" sao sempre os mesmos. */
static unsigned long proximo_aleatorio(unsigned long *estado)
{
    *estado = (*estado * 1103515245UL + 12345UL) & 0x7fffffffUL;
    return *estado;
}

static Grafo *montar_aleatorio(EstruturaGrafo estrutura, size_t n,
    unsigned int densidade_percentual, unsigned long semente)
{
    Grafo *grafo = criar_com_vertices(estrutura, n);
    size_t a, b;

    for (a = 0U; a < n; ++a)
        for (b = a + 1U; b < n; ++b)
            if (proximo_aleatorio(&semente) % 100U < densidade_percentual)
                assert(grafo_adicionar_aresta(grafo, a, b) == 1);
    return grafo;
}

static size_t cores_exata(const Grafo *grafo)
{
    ResultadoColoracao resultado;
    size_t total;

    assert(coloracao_exata(grafo, NULL, &resultado, NULL) == COLORACAO_EXATA_OK);
    assert(coloracao_valida(grafo, &resultado) == 1);
    total = resultado.total_cores;
    coloracao_liberar(&resultado);
    return total;
}

/* Executa a gulosa na ordem pedida, exige que seja valida e devolve as cores. */
static size_t cores_gulosa(const Grafo *grafo, OrdemColoracao ordem)
{
    ResultadoColoracao resultado;
    size_t total;

    assert(coloracao_gulosa_ordem(grafo, ordem, &resultado) == 1);
    assert(coloracao_valida(grafo, &resultado) == 1);
    total = resultado.total_cores;
    coloracao_liberar(&resultado);
    return total;
}

static size_t grau_maximo(const Grafo *grafo)
{
    size_t graus[64], i, maximo = 0U;

    assert(grafo_quantidade_vertices(grafo) <= 64U);
    assert(coloracao_calcular_graus(grafo, graus) == 1);
    for (i = 0U; i < grafo_quantidade_vertices(grafo); ++i)
        if (graus[i] > maximo) maximo = graus[i];
    return maximo;
}

/* Exata com o valor esperado; gulosa (nas duas ordens) valida e >= exata. */
static void conferir(Grafo *grafo, size_t numero_cromatico)
{
    size_t exata = cores_exata(grafo);

    assert(exata == numero_cromatico);
    assert(cores_gulosa(grafo, COLORACAO_ORDEM_NATURAL) >= exata);
    assert(cores_gulosa(grafo, COLORACAO_WELSH_POWELL) >= exata);
    grafo_destruir(grafo);
}

static void teste_bipartidos_com_duas_cores(EstruturaGrafo estrutura)
{
    size_t m, n;
    Grafo *coroa;

    for (m = 1U; m <= 4U; ++m)
        for (n = 1U; n <= 4U; ++n) conferir(montar_bipartido_completo(estrutura, m, n), 2U);
    for (n = 4U; n <= 12U; n += 2U) conferir(montar_ciclo(estrutura, n), 2U);
    conferir(montar_grade(estrutura, 3U, 4U), 2U);
    conferir(montar_grade(estrutura, 1U, 6U), 2U);
    for (n = 3U; n <= 5U; ++n) conferir(montar_coroa(estrutura, n), 2U);

    /* Na coroa de 2 x 5 vertices a gulosa natural precisa de 5 cores. */
    coroa = montar_coroa(estrutura, 5U);
    assert(cores_gulosa(coroa, COLORACAO_ORDEM_NATURAL) == 5U);
    grafo_destruir(coroa);
}

static void teste_completos_com_n_cores(EstruturaGrafo estrutura)
{
    size_t n;
    Grafo *grafo;

    for (n = 1U; n <= 8U; ++n) {
        grafo = montar_completo(estrutura, n);
        /* Em Kn todas as ordens dao exatamente n cores. */
        assert(cores_gulosa(grafo, COLORACAO_ORDEM_NATURAL) == n);
        assert(cores_gulosa(grafo, COLORACAO_WELSH_POWELL) == n);
        conferir(grafo, n);
    }
}

static void teste_ciclos_impares_com_tres_cores(EstruturaGrafo estrutura)
{
    size_t n;

    for (n = 3U; n <= 13U; n += 2U) conferir(montar_ciclo(estrutura, n), 3U);
}

/* Grafos aleatorios: nao sabemos o numero cromatico de antemao, mas a gulosa
 * precisa ser valida e usar pelo menos tantas cores quanto a exata.
 */
static void teste_gulosa_valida_e_maior_ou_igual(EstruturaGrafo estrutura)
{
    static const unsigned int densidades[] = {10U, 30U, 50U, 70U, 90U};
    unsigned long semente;
    size_t d, n, exata, natural, welsh_powell, comparacoes = 0U;
    Grafo *grafo;

    for (d = 0U; d < sizeof(densidades) / sizeof(densidades[0]); ++d) {
        for (n = 1U; n <= 12U; ++n) {
            for (semente = 1U; semente <= 4U; ++semente) {
                grafo = montar_aleatorio(estrutura, n, densidades[d], semente * 7919UL + n);
                exata = cores_exata(grafo);
                natural = cores_gulosa(grafo, COLORACAO_ORDEM_NATURAL);
                welsh_powell = cores_gulosa(grafo, COLORACAO_WELSH_POWELL);
                assert(natural >= exata && welsh_powell >= exata);
                /* Limite classico da gulosa: no maximo grau maximo + 1 cores. */
                assert(natural <= grau_maximo(grafo) + 1U);
                assert(welsh_powell <= grau_maximo(grafo) + 1U);
                grafo_destruir(grafo);
                ++comparacoes;
            }
        }
    }
    assert(comparacoes == 5U * 12U * 4U);
}

int main(void)
{
    size_t i;

    for (i = 0U; i < QUANTIDADE_ESTRUTURAS; ++i) {
        teste_bipartidos_com_duas_cores(ESTRUTURAS[i]);
        teste_completos_com_n_cores(ESTRUTURAS[i]);
        teste_ciclos_impares_com_tres_cores(ESTRUTURAS[i]);
        teste_gulosa_valida_e_maior_ou_igual(ESTRUTURAS[i]);
    }
    puts("Testes das propriedades da coloracao passaram.");
    return 0;
}
