#include <assert.h>
#include <stdio.h>

#include "interferencia.h"

/* Um grau de latitude vale cerca de 111,19 km em qualquer lugar do globo, o
 * que torna previsivel a distancia entre antenas dispostas em coluna. Com
 * alcance de 1.000 m em cada antena, a soma dos alcances e 2 km: 0,01 grau
 * (1,112 km) se sobrepoe e 0,02 grau (2,224 km) nao. */
#define GRAU_EM_KM 111.1949
#define ALCANCE_PADRAO_M 1000.0

static Grafo *criar(EstruturaGrafo estrutura)
{
    Grafo *grafo = grafo_criar_com_estrutura(estrutura);
    assert(grafo != NULL);
    return grafo;
}

/* Antena da operadora padrao do dataset: mcc 724 (Brasil), net 2, area 1. */
static void antena(Grafo *grafo, size_t indice, double latitude,
                   double longitude)
{
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 1U, (unsigned int)indice,
        latitude, longitude, ALCANCE_PADRAO_M) == (int)indice);
}

static void antena_operadora(Grafo *grafo, size_t indice, unsigned int mcc,
                             unsigned int net, unsigned int area,
                             double latitude, double longitude)
{
    assert(grafo_adicionar_antena(grafo, mcc, net, area, (unsigned int)indice,
        latitude, longitude, ALCANCE_PADRAO_M) == (int)indice);
}

static void antena_alcance(Grafo *grafo, size_t indice, double latitude,
                           double longitude, double alcance_metros)
{
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 1U, (unsigned int)indice,
        latitude, longitude, alcance_metros) == (int)indice);
}

/* Duas antenas a 0,01 grau (cerca de 1,11 km) com 1 km de alcance cada: a
 * soma dos alcances e 2 km, os sinais se sobrepoem e ha interferencia. */
static void teste_par_que_se_sobrepoe(EstruturaGrafo estrutura)
{
    Grafo *grafo = criar(estrutura);
    RelatorioInterferencia relatorio;

    antena(grafo, 0U, -23.0, -46.0);
    antena(grafo, 1U, -23.01, -46.0);

    assert(interferencia_construir(grafo, &relatorio));
    assert(relatorio.vertices == 2U);
    assert(relatorio.arestas == 1U);
    assert(relatorio.grau_maximo == 1U);
    assert(relatorio.vertices_isolados == 0U);
    /* A aresta e nao direcionada: responde nos dois sentidos. */
    assert(grafo_sao_adjacentes(grafo, 0U, 1U));
    assert(grafo_sao_adjacentes(grafo, 1U, 0U));
    grafo_destruir(grafo);
}

/* As mesmas duas antenas a 0,02 grau (cerca de 2,22 km): passa dos 2 km da
 * soma dos alcances, os sinais nao se encontram e nao ha aresta. */
static void teste_par_que_nao_se_sobrepoe(EstruturaGrafo estrutura)
{
    Grafo *grafo = criar(estrutura);
    RelatorioInterferencia relatorio;

    antena(grafo, 0U, -23.0, -46.0);
    antena(grafo, 1U, -23.02, -46.0);

    assert(interferencia_construir(grafo, &relatorio));
    assert(relatorio.vertices == 2U);
    assert(relatorio.arestas == 0U);
    assert(relatorio.grau_maximo == 0U);
    assert(relatorio.vertices_isolados == 2U);
    assert(!grafo_sao_adjacentes(grafo, 0U, 1U));
    grafo_destruir(grafo);
}

/* O limite e inclusivo: d(u,v) <= alcance(u) + alcance(v). Duas antenas
 * separadas por exatamente a soma dos alcances interferem. */
static void teste_limite_inclusivo(void)
{
    Grafo *grafo = criar(GRAFO_ESTRUTURA_CONJUNTA);
    RelatorioInterferencia relatorio;
    /* 0,01 grau de latitude em metros: e essa a distancia entre as duas. */
    double distancia_m = GRAU_EM_KM * 0.01 * 1000.0;

    /* Metade da distancia em cada antena: a soma bate com a distancia, a
     * menos do erro do valor de GRAU_EM_KM, descontado com uma folga de 1 m. */
    antena_alcance(grafo, 0U, -23.0, -46.0, distancia_m / 2.0 + 1.0);
    antena_alcance(grafo, 1U, -23.01, -46.0, distancia_m / 2.0 + 1.0);

    assert(interferencia_construir(grafo, &relatorio));
    assert(relatorio.arestas == 1U);
    grafo_destruir(grafo);
}

/* Torres de operadoras diferentes nao interferem: cada operadora tem sua
 * faixa de espectro. Tres antenas no mesmo ponto, uma por operadora, nao
 * geram aresta nenhuma apesar da distancia zero. */
static void teste_operadoras_diferentes(EstruturaGrafo estrutura)
{
    Grafo *grafo = criar(estrutura);
    RelatorioInterferencia relatorio;

    antena_operadora(grafo, 0U, 724U, 2U, 1U, -23.0, -46.0);
    antena_operadora(grafo, 1U, 724U, 3U, 1U, -23.0, -46.0); /* outro net */
    antena_operadora(grafo, 2U, 724U, 2U, 2U, -23.0, -46.0); /* outra area */
    antena_operadora(grafo, 3U, 722U, 2U, 1U, -23.0, -46.0); /* outro mcc */

    assert(interferencia_construir(grafo, &relatorio));
    assert(relatorio.vertices == 4U);
    assert(relatorio.arestas == 0U);
    assert(relatorio.grau_maximo == 0U);
    assert(relatorio.vertices_isolados == 4U);
    grafo_destruir(grafo);
}

/* Duas antenas da mesma operadora no mesmo ponto interferem; a terceira, de
 * outra operadora e tambem no mesmo ponto, nao entra. Confirma que o filtro
 * de operadora nao esta descartando pares validos junto com os invalidos. */
static void teste_mesma_operadora_entre_vizinhas_de_outras(void)
{
    Grafo *grafo = criar(GRAFO_ESTRUTURA_CONJUNTA);
    RelatorioInterferencia relatorio;

    antena_operadora(grafo, 0U, 724U, 2U, 1U, -23.0, -46.0);
    antena_operadora(grafo, 1U, 724U, 3U, 1U, -23.0, -46.0);
    antena_operadora(grafo, 2U, 724U, 2U, 1U, -23.0, -46.0);

    assert(interferencia_construir(grafo, &relatorio));
    assert(relatorio.arestas == 1U);
    assert(grafo_sao_adjacentes(grafo, 0U, 2U));
    assert(!grafo_sao_adjacentes(grafo, 0U, 1U));
    assert(relatorio.vertices_isolados == 1U);
    grafo_destruir(grafo);
}

/* Uma antena central alcançada por quatro vizinhas que nao se alcançam entre
 * si: estrela de grau 4. E o caso que a issue #69 pede para relatar, porque
 * Delta + 1 = 5 e o limite de canais da coloracao gulosa. */
static void teste_grau_maximo(EstruturaGrafo estrutura)
{
    Grafo *grafo = criar(estrutura);
    RelatorioInterferencia relatorio;

    /* A central tem alcance grande; as pontas, alcance pequeno. Assim cada
     * ponta se sobrepoe a central mas nao as outras pontas. */
    antena_alcance(grafo, 0U, -23.0, -46.0, 20000.0);
    antena_alcance(grafo, 1U, -22.9, -46.0, 100.0);
    antena_alcance(grafo, 2U, -23.1, -46.0, 100.0);
    antena_alcance(grafo, 3U, -23.0, -45.9, 100.0);
    antena_alcance(grafo, 4U, -23.0, -46.1, 100.0);

    assert(interferencia_construir(grafo, &relatorio));
    assert(relatorio.vertices == 5U);
    assert(relatorio.arestas == 4U);
    assert(relatorio.grau_maximo == 4U);
    assert(relatorio.vertice_grau_maximo == 0U);
    assert(relatorio.vertices_isolados == 0U);
    grafo_destruir(grafo);
}

/* Uma antena longe de todas as outras nao interfere em ninguem: pode usar
 * qualquer canal. */
static void teste_antena_isolada(EstruturaGrafo estrutura)
{
    Grafo *grafo = criar(estrutura);
    RelatorioInterferencia relatorio;

    antena(grafo, 0U, -23.0, -46.0);
    antena(grafo, 1U, -23.01, -46.0);
    antena(grafo, 2U, 10.0, 10.0);

    assert(interferencia_construir(grafo, &relatorio));
    assert(relatorio.vertices == 3U);
    assert(relatorio.arestas == 1U);
    assert(relatorio.grau_maximo == 1U);
    assert(relatorio.vertices_isolados == 1U);
    grafo_destruir(grafo);
}

static void teste_parametros_invalidos(void)
{
    Grafo *grafo = criar(GRAFO_ESTRUTURA_CONJUNTA);
    RelatorioInterferencia relatorio;

    antena(grafo, 0U, -23.0, -46.0);
    antena(grafo, 1U, -23.01, -46.0);

    assert(!interferencia_construir(NULL, &relatorio));
    assert(!interferencia_construir(grafo, NULL));
    /* Nenhuma tentativa invalida pode ter criado aresta. */
    assert(grafo_quantidade_arestas(grafo) == 0U);

    /* Com arestas ja construidas, a funcao recusa em vez de misturar o
     * criterio da Fase I, o do cabeamento e o da interferencia no mesmo
     * grafo. */
    assert(grafo_adicionar_aresta(grafo, 0U, 1U));
    assert(!interferencia_construir(grafo, &relatorio));
    assert(grafo_quantidade_arestas(grafo) == 1U);
    grafo_destruir(grafo);
}

static void teste_grafos_minusculos(void)
{
    Grafo *vazio = criar(GRAFO_ESTRUTURA_CONJUNTA);
    Grafo *unico = criar(GRAFO_ESTRUTURA_CONJUNTA);
    RelatorioInterferencia relatorio;

    assert(interferencia_construir(vazio, &relatorio));
    assert(relatorio.vertices == 0U && relatorio.arestas == 0U);
    assert(relatorio.grau_maximo == 0U && relatorio.vertices_isolados == 0U);

    antena(unico, 0U, -23.0, -46.0);
    assert(interferencia_construir(unico, &relatorio));
    assert(relatorio.vertices == 1U && relatorio.arestas == 0U);
    assert(relatorio.grau_maximo == 0U && relatorio.vertices_isolados == 1U);

    grafo_destruir(vazio);
    grafo_destruir(unico);
}

/* A issue #69 exige funcionar com pelo menos 1.000 vertices.
 *
 * Malha de 40 x 25 antenas espacadas 0,02 grau. Com 2 km de alcance em cada
 * antena, a soma e 4 km e alcança as vizinhas ortogonais (2,22 km na vertical
 * e 2,05 km na horizontal, nesta latitude) e as diagonais (3,02 km), mas nao
 * os saltos de duas posicoes (4,10 km na horizontal, 4,45 km na vertical).
 *
 * Arestas esperadas: 25 x 39 = 975 horizontais, 40 x 24 = 960 verticais e
 * 2 x 39 x 24 = 1.872 diagonais, somando 3.807. O grau maximo e 8: as oito
 * vizinhas de uma antena interna da malha.
 */
static void teste_mil_vertices(void)
{
    Grafo *grafo = criar(GRAFO_ESTRUTURA_CONJUNTA);
    RelatorioInterferencia relatorio;
    size_t i;

    for (i = 0U; i < 1000U; ++i)
        antena_alcance(grafo, i, -23.0 + (double)(i / 40U) * 0.02,
            -46.0 + (double)(i % 40U) * 0.02, 2000.0);

    assert(interferencia_construir(grafo, &relatorio));
    assert(relatorio.vertices == 1000U);
    assert(relatorio.arestas == 975U + 960U + 1872U);
    assert(relatorio.grau_maximo == 8U);
    assert(relatorio.vertices_isolados == 0U);
    grafo_destruir(grafo);
}

/* A lista e a matriz de adjacencia tem que chegar ao mesmo grafo: mesma
 * quantidade de arestas e mesmo grau maximo. E o que a issue #69 pede com
 * "funcionar com Lista e Matriz de Adjacencia". */
static void teste_estruturas_concordam(void)
{
    EstruturaGrafo estruturas[3] = {GRAFO_ESTRUTURA_CONJUNTA,
        GRAFO_LISTA_ADJACENCIA, GRAFO_MATRIZ_ADJACENCIA};
    RelatorioInterferencia primeiro;
    size_t e;

    for (e = 0U; e < 3U; ++e) {
        Grafo *grafo = criar(estruturas[e]);
        RelatorioInterferencia relatorio;
        size_t i;

        /* Malha de 10 x 10 com alcance de 2 km, mesma geometria do teste
         * grande em escala menor. */
        for (i = 0U; i < 100U; ++i)
            antena_alcance(grafo, i, -23.0 + (double)(i / 10U) * 0.02,
                -46.0 + (double)(i % 10U) * 0.02, 2000.0);

        assert(interferencia_construir(grafo, &relatorio));
        if (e == 0U) {
            primeiro = relatorio;
            assert(primeiro.arestas > 0U);
            assert(primeiro.grau_maximo == 8U);
        } else {
            assert(relatorio.vertices == primeiro.vertices);
            assert(relatorio.arestas == primeiro.arestas);
            assert(relatorio.grau_maximo == primeiro.grau_maximo);
            assert(relatorio.vertices_isolados == primeiro.vertices_isolados);
        }
        grafo_destruir(grafo);
    }
}

int main(void)
{
    teste_par_que_se_sobrepoe(GRAFO_ESTRUTURA_CONJUNTA);
    teste_par_que_se_sobrepoe(GRAFO_LISTA_ADJACENCIA);
    teste_par_que_se_sobrepoe(GRAFO_MATRIZ_ADJACENCIA);
    teste_par_que_nao_se_sobrepoe(GRAFO_ESTRUTURA_CONJUNTA);
    teste_par_que_nao_se_sobrepoe(GRAFO_LISTA_ADJACENCIA);
    teste_par_que_nao_se_sobrepoe(GRAFO_MATRIZ_ADJACENCIA);
    teste_limite_inclusivo();
    teste_operadoras_diferentes(GRAFO_ESTRUTURA_CONJUNTA);
    teste_operadoras_diferentes(GRAFO_LISTA_ADJACENCIA);
    teste_operadoras_diferentes(GRAFO_MATRIZ_ADJACENCIA);
    teste_mesma_operadora_entre_vizinhas_de_outras();
    teste_grau_maximo(GRAFO_ESTRUTURA_CONJUNTA);
    teste_grau_maximo(GRAFO_LISTA_ADJACENCIA);
    teste_grau_maximo(GRAFO_MATRIZ_ADJACENCIA);
    teste_antena_isolada(GRAFO_ESTRUTURA_CONJUNTA);
    teste_antena_isolada(GRAFO_LISTA_ADJACENCIA);
    teste_antena_isolada(GRAFO_MATRIZ_ADJACENCIA);
    teste_parametros_invalidos();
    teste_grafos_minusculos();
    teste_mil_vertices();
    teste_estruturas_concordam();
    puts("Testes do grafo de interferencia passaram.");
    return 0;
}