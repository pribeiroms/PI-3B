#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "candidato_fibra.h"

/* Um grau de latitude vale cerca de 111,19 km em qualquer lugar do globo, o
 * que torna previsivel a distancia entre antenas dispostas em coluna. */
#define GRAU_EM_KM 111.1949

static Grafo *criar(EstruturaGrafo estrutura)
{
    Grafo *grafo = grafo_criar_com_estrutura(estrutura);
    assert(grafo != NULL);
    return grafo;
}

static void antena(Grafo *grafo, size_t indice, double latitude,
                   double longitude)
{
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 1U, (unsigned int)indice,
        latitude, longitude, 1000.0) == (int)indice);
}

/* Quatro antenas em coluna, espacadas 0,1 grau (cerca de 11,1 km). */
static Grafo *criar_coluna(EstruturaGrafo estrutura, size_t quantidade)
{
    Grafo *grafo = criar(estrutura);
    size_t i;
    for (i = 0U; i < quantidade; ++i)
        antena(grafo, i, -23.0 + (double)i * 0.1, -46.0);
    return grafo;
}

static ParametrosCandidatoFibra parametros(size_t k, double raio_km)
{
    ParametrosCandidatoFibra p;
    p.k = k;
    p.raio_maximo_km = raio_km;
    return p;
}

/* Com k = 1 numa coluna de quatro antenas, cada uma escolhe a vizinha
 * imediata: 0-1, 1-0 (ja existe), 2-1, 3-2. Restam tres arestas e um unico
 * componente -- exatamente a floresta que a Fase I produziria. */
static void teste_k_igual_a_um(EstruturaGrafo estrutura)
{
    Grafo *grafo = criar_coluna(estrutura, 4U);
    RelatorioCandidatoFibra relatorio;
    double peso = 0.0;

    assert(candidato_fibra_construir(grafo, parametros(1U, 50.0), &relatorio));
    assert(relatorio.vertices == 4U);
    assert(relatorio.arestas == 3U);
    assert(relatorio.componentes == 1U);
    assert(relatorio.vertices_isolados == 0U);
    /* O peso e a distancia geodesica, nao um indice qualquer. */
    assert(grafo_obter_peso(grafo, 0U, 1U, &peso));
    assert(fabs(peso - GRAU_EM_KM * 0.1) < 0.01);
    assert(fabs(relatorio.maior_aresta_km - GRAU_EM_KM * 0.1) < 0.01);
    assert(fabs(relatorio.peso_total_km - 3.0 * GRAU_EM_KM * 0.1) < 0.03);
    grafo_destruir(grafo);
}

/* Com k = 2 cada antena alcança tambem a segunda vizinha, o que fecha ciclos:
 * e esse excedente que da ao Kruskal rotas redundantes para descartar. */
static void teste_k_maior_gera_mais_rotas(void)
{
    Grafo *um = criar_coluna(GRAFO_ESTRUTURA_CONJUNTA, 4U);
    Grafo *dois = criar_coluna(GRAFO_ESTRUTURA_CONJUNTA, 4U);
    RelatorioCandidatoFibra relatorio_um, relatorio_dois;

    assert(candidato_fibra_construir(um, parametros(1U, 50.0), &relatorio_um));
    assert(candidato_fibra_construir(dois, parametros(2U, 50.0), &relatorio_dois));
    assert(relatorio_dois.arestas > relatorio_um.arestas);
    /* 3 arestas da coluna + 2 saltos de duas posicoes (0-2 e 1-3). */
    assert(relatorio_dois.arestas == 5U);
    assert(relatorio_dois.componentes == 1U);
    grafo_destruir(um);
    grafo_destruir(dois);
}

/* O grau pode passar de k: um vertice central e escolhido por todos os
 * vizinhos que o tem entre os k mais proximos deles. */
static void teste_grau_pode_passar_de_k(void)
{
    Grafo *grafo = criar(GRAFO_LISTA_ADJACENCIA);
    RelatorioCandidatoFibra relatorio;
    const NoAdjacencia *vizinho;
    size_t grau = 0U;

    /* Uma antena central e quatro ao redor, todas mais proximas do centro do
     * que umas das outras. */
    antena(grafo, 0U, -23.0, -46.0);
    antena(grafo, 1U, -22.9, -46.0);
    antena(grafo, 2U, -23.1, -46.0);
    antena(grafo, 3U, -23.0, -45.89);
    antena(grafo, 4U, -23.0, -46.11);

    assert(candidato_fibra_construir(grafo, parametros(1U, 50.0), &relatorio));
    for (vizinho = grafo_vizinhos(grafo, 0U); vizinho != NULL;
         vizinho = vizinho->proximo) ++grau;
    assert(grau > 1U);
    assert(relatorio.componentes == 1U);
    grafo_destruir(grafo);
}

/* O raio maximo separa grupos distantes, e a contagem de componentes e o que
 * revela isso. */
static void teste_raio_separa_componentes(void)
{
    Grafo *grafo = criar(GRAFO_ESTRUTURA_CONJUNTA);
    RelatorioCandidatoFibra relatorio;

    /* Dois pares proximos entre si, mas a um grau de distancia um do outro. */
    antena(grafo, 0U, -23.0, -46.0);
    antena(grafo, 1U, -23.01, -46.0);
    antena(grafo, 2U, -24.0, -46.0);
    antena(grafo, 3U, -24.01, -46.0);

    assert(candidato_fibra_construir(grafo, parametros(2U, 50.0), &relatorio));
    assert(relatorio.arestas == 2U);
    assert(relatorio.componentes == 2U);
    assert(relatorio.vertices_isolados == 0U);
    grafo_destruir(grafo);
}

/* Uma antena longe de todas as outras nao recebe rota nenhuma e forma um
 * componente so dela. */
static void teste_antena_isolada(void)
{
    Grafo *grafo = criar(GRAFO_ESTRUTURA_CONJUNTA);
    RelatorioCandidatoFibra relatorio;

    antena(grafo, 0U, -23.0, -46.0);
    antena(grafo, 1U, -23.01, -46.0);
    antena(grafo, 2U, 10.0, 10.0);

    assert(candidato_fibra_construir(grafo, parametros(4U, 50.0), &relatorio));
    assert(relatorio.arestas == 1U);
    assert(relatorio.componentes == 2U);
    assert(relatorio.vertices_isolados == 1U);
    grafo_destruir(grafo);
}

static void teste_parametros_invalidos(void)
{
    Grafo *grafo = criar_coluna(GRAFO_ESTRUTURA_CONJUNTA, 3U);
    RelatorioCandidatoFibra relatorio;

    assert(!candidato_fibra_construir(NULL, parametros(1U, 50.0), &relatorio));
    assert(!candidato_fibra_construir(grafo, parametros(1U, 50.0), NULL));
    assert(!candidato_fibra_construir(grafo, parametros(0U, 50.0), &relatorio));
    assert(!candidato_fibra_construir(grafo, parametros(1U, 0.0), &relatorio));
    assert(!candidato_fibra_construir(grafo, parametros(1U, -5.0), &relatorio));
    assert(!candidato_fibra_construir(grafo, parametros(1U, INFINITY), &relatorio));
    assert(!candidato_fibra_construir(grafo, parametros(1U, NAN), &relatorio));
    /* Nenhuma tentativa invalida pode ter criado aresta. */
    assert(grafo_quantidade_arestas(grafo) == 0U);

    /* Com arestas ja construidas, a funcao recusa em vez de misturar os dois
     * criterios no mesmo grafo. */
    assert(grafo_adicionar_aresta(grafo, 0U, 2U));
    assert(!candidato_fibra_construir(grafo, parametros(1U, 50.0), &relatorio));
    assert(grafo_quantidade_arestas(grafo) == 1U);
    grafo_destruir(grafo);
}

static void teste_grafos_minusculos(void)
{
    Grafo *vazio = criar(GRAFO_ESTRUTURA_CONJUNTA);
    Grafo *unico = criar_coluna(GRAFO_ESTRUTURA_CONJUNTA, 1U);
    RelatorioCandidatoFibra relatorio;

    assert(candidato_fibra_construir(vazio, parametros(4U, 50.0), &relatorio));
    assert(relatorio.vertices == 0U && relatorio.arestas == 0U);
    assert(relatorio.componentes == 0U);

    assert(candidato_fibra_construir(unico, parametros(4U, 50.0), &relatorio));
    assert(relatorio.vertices == 1U && relatorio.arestas == 0U);
    assert(relatorio.componentes == 1U && relatorio.vertices_isolados == 1U);

    /* k maior que a quantidade de vizinhos disponiveis nao e erro. */
    grafo_destruir(vazio);
    grafo_destruir(unico);
}

/* A issue #68 exige suportar pelo menos 1.000 vertices. */
static void teste_mil_vertices(void)
{
    Grafo *grafo = criar(GRAFO_ESTRUTURA_CONJUNTA);
    RelatorioCandidatoFibra relatorio;
    size_t i;

    /* Malha de 40 x 25 antenas espacadas 0,02 grau (cerca de 2,2 km). */
    for (i = 0U; i < 1000U; ++i)
        antena(grafo, i, -23.0 + (double)(i / 40U) * 0.02,
            -46.0 + (double)(i % 40U) * 0.02);

    assert(candidato_fibra_construir(grafo,
        candidato_fibra_parametros_padrao(), &relatorio));
    assert(relatorio.vertices == 1000U);
    /* Numa malha, as quatro vizinhas mais proximas de uma antena interna sao
     * as ortogonais: 39 x 25 horizontais + 40 x 24 verticais = 1.935 rotas.
     * As antenas da borda tem menos de quatro vizinhas ortogonais e gastam as
     * escolhas restantes em diagonais: sao 4 x 1.000 - 2 x 1.935 = 130 dessas,
     * nenhuma reciproca, o que fecha 2.065. */
    assert(relatorio.arestas == 1935U + 130U);
    assert(relatorio.componentes == 1U);
    assert(relatorio.vertices_isolados == 0U);
    assert(relatorio.peso_total_km > 0.0);
    /* Nenhuma rota passa de dois passos da malha: o criterio nao alcançou
     * antenas distantes. */
    assert(relatorio.maior_aresta_km < 4.2);
    grafo_destruir(grafo);
}

static void teste_contagem_de_componentes(EstruturaGrafo estrutura)
{
    Grafo *grafo = criar_coluna(estrutura, 4U);
    size_t componentes = 99U;

    assert(grafo_contar_componentes(grafo, &componentes));
    assert(componentes == 4U);
    assert(grafo_adicionar_aresta(grafo, 0U, 1U));
    assert(grafo_contar_componentes(grafo, &componentes));
    assert(componentes == 3U);
    assert(grafo_adicionar_aresta(grafo, 2U, 3U));
    assert(grafo_adicionar_aresta(grafo, 1U, 2U));
    assert(grafo_contar_componentes(grafo, &componentes));
    assert(componentes == 1U);
    assert(!grafo_contar_componentes(NULL, &componentes));
    assert(!grafo_contar_componentes(grafo, NULL));
    grafo_destruir(grafo);
}

int main(void)
{
    teste_k_igual_a_um(GRAFO_ESTRUTURA_CONJUNTA);
    teste_k_igual_a_um(GRAFO_LISTA_ADJACENCIA);
    teste_k_igual_a_um(GRAFO_MATRIZ_ADJACENCIA);
    teste_k_maior_gera_mais_rotas();
    teste_grau_pode_passar_de_k();
    teste_raio_separa_componentes();
    teste_antena_isolada();
    teste_parametros_invalidos();
    teste_grafos_minusculos();
    teste_mil_vertices();
    teste_contagem_de_componentes(GRAFO_ESTRUTURA_CONJUNTA);
    teste_contagem_de_componentes(GRAFO_LISTA_ADJACENCIA);
    teste_contagem_de_componentes(GRAFO_MATRIZ_ADJACENCIA);
    puts("Testes do grafo candidato de cabeamento passaram.");
    return 0;
}