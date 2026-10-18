#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "grafo.h"

static Grafo *criar_com_antenas(EstruturaGrafo estrutura, size_t quantidade)
{
    Grafo *grafo = grafo_criar_com_estrutura(estrutura);
    size_t i;

    assert(grafo != NULL);
    for (i = 0U; i < quantidade; ++i) {
        double deslocamento = (double)i * 0.01;
        assert(grafo_adicionar_antena(grafo, 724U, 2U, 1U, (unsigned int)i,
            -23.0 + deslocamento, -46.0 + deslocamento, 1000.0) == (int)i);
    }
    return grafo;
}

/* O mesmo roteiro e executado nas tres estruturas: a alternancia entre lista e
 * matriz nao pode alterar o resultado observado pela API. */
static void teste_inserir_e_consultar(EstruturaGrafo estrutura)
{
    Grafo *grafo = criar_com_antenas(estrutura, 4U);
    double peso = -1.0;

    assert(grafo_adicionar_aresta_ponderada(grafo, 0U, 1U, 2.5));
    assert(grafo_adicionar_aresta_ponderada(grafo, 1U, 2U, 7.25));

    assert(grafo_obter_peso(grafo, 0U, 1U, &peso));
    assert(peso == 2.5);
    /* O grafo e nao direcionado: o peso e o mesmo nos dois sentidos. */
    assert(grafo_obter_peso(grafo, 1U, 0U, &peso));
    assert(peso == 2.5);
    assert(grafo_obter_peso(grafo, 1U, 2U, &peso));
    assert(peso == 7.25);

    /* Aresta inexistente nao devolve peso. */
    peso = -1.0;
    assert(!grafo_obter_peso(grafo, 0U, 3U, &peso));
    assert(peso == -1.0);
    /* Laço e indice fora do intervalo. */
    assert(!grafo_obter_peso(grafo, 0U, 0U, &peso));
    assert(!grafo_obter_peso(grafo, 0U, 9U, &peso));
    assert(!grafo_obter_peso(NULL, 0U, 1U, &peso));
    assert(!grafo_obter_peso(grafo, 0U, 1U, NULL));

    /* A adjacencia continua valendo. */
    assert(grafo_sao_adjacentes(grafo, 0U, 1U));
    assert(!grafo_sao_adjacentes(grafo, 0U, 3U));
    assert(grafo_quantidade_arestas(grafo) == 2U);

    grafo_destruir(grafo);
}

static void teste_vizinhos_trazem_peso(void)
{
    /* A lista de adjacencia so existe fora do modo matriz. */
    Grafo *grafo = criar_com_antenas(GRAFO_LISTA_ADJACENCIA, 3U);
    const NoAdjacencia *vizinho;
    int encontrou = 0;

    assert(grafo_adicionar_aresta_ponderada(grafo, 0U, 1U, 3.75));
    assert(grafo_adicionar_aresta_ponderada(grafo, 0U, 2U, 1.25));
    for (vizinho = grafo_vizinhos(grafo, 0U); vizinho != NULL;
         vizinho = vizinho->proximo) {
        if (vizinho->vertice == 1U) { assert(vizinho->peso == 3.75); ++encontrou; }
        if (vizinho->vertice == 2U) { assert(vizinho->peso == 1.25); ++encontrou; }
    }
    assert(encontrou == 2);
    grafo_destruir(grafo);
}

static void teste_percorrer_arestas(void)
{
    Grafo *grafo = criar_com_antenas(GRAFO_ESTRUTURA_CONJUNTA, 3U);
    size_t origem, destino;
    double peso;

    assert(grafo_adicionar_aresta_ponderada(grafo, 0U, 1U, 4.0));
    assert(grafo_adicionar_aresta_ponderada(grafo, 1U, 2U, 6.0));

    assert(grafo_obter_aresta_ponderada(grafo, 0U, &origem, &destino, &peso));
    assert(origem == 0U && destino == 1U && peso == 4.0);
    assert(grafo_obter_aresta_ponderada(grafo, 1U, &origem, &destino, &peso));
    assert(origem == 1U && destino == 2U && peso == 6.0);
    /* Ponteiros de saida sao opcionais. */
    assert(grafo_obter_aresta_ponderada(grafo, 0U, NULL, NULL, &peso));
    assert(peso == 4.0);
    assert(grafo_obter_aresta_ponderada(grafo, 0U, NULL, NULL, NULL));
    /* Indice fora do intervalo. */
    assert(!grafo_obter_aresta_ponderada(grafo, 2U, &origem, &destino, &peso));

    assert(grafo_peso_total(grafo) == 10.0);
    assert(grafo_peso_total(NULL) == 0.0);

    grafo_destruir(grafo);
}

static void teste_pesos_invalidos(void)
{
    Grafo *grafo = criar_com_antenas(GRAFO_ESTRUTURA_CONJUNTA, 3U);

    assert(!grafo_adicionar_aresta_ponderada(grafo, 0U, 1U, -1.0));
    assert(!grafo_adicionar_aresta_ponderada(grafo, 0U, 1U, INFINITY));
    assert(!grafo_adicionar_aresta_ponderada(grafo, 0U, 1U, NAN));
    /* Nenhuma delas pode ter sido registrada. */
    assert(grafo_quantidade_arestas(grafo) == 0U);

    /* Laço e aresta repetida continuam rejeitados. */
    assert(!grafo_adicionar_aresta_ponderada(grafo, 0U, 0U, 1.0));
    assert(grafo_adicionar_aresta_ponderada(grafo, 0U, 1U, 1.0));
    assert(!grafo_adicionar_aresta_ponderada(grafo, 0U, 1U, 2.0));
    assert(!grafo_adicionar_aresta_ponderada(grafo, 1U, 0U, 2.0));
    assert(grafo_quantidade_arestas(grafo) == 1U);

    grafo_destruir(grafo);
}

static void teste_compatibilidade_com_a_fase_um(void)
{
    /* grafo_adicionar_aresta continua funcionando e registra peso zero. */
    Grafo *grafo = criar_com_antenas(GRAFO_ESTRUTURA_CONJUNTA, 3U);
    double peso = -1.0;

    assert(grafo_adicionar_aresta(grafo, 0U, 1U));
    assert(grafo_obter_peso(grafo, 0U, 1U, &peso));
    assert(peso == 0.0);
    assert(grafo_peso_total(grafo) == 0.0);
    assert(grafo_sao_adjacentes(grafo, 0U, 1U));
    grafo_destruir(grafo);
}

static void teste_pesos_sobrevivem_a_expansao(void)
{
    /* A capacidade inicial e de 128 posicoes; com 300 vertices a matriz e as
     * listas sao expandidas, e os pesos ja gravados devem acompanhar. */
    Grafo *grafo = criar_com_antenas(GRAFO_ESTRUTURA_CONJUNTA, 300U);
    double peso;
    size_t i;

    assert(grafo_adicionar_aresta_ponderada(grafo, 0U, 1U, 1.5));
    for (i = 10U; i < 290U; i += 2U)
        assert(grafo_adicionar_aresta_ponderada(grafo, i, i + 1U,
            (double)i / 4.0));

    assert(grafo_obter_peso(grafo, 0U, 1U, &peso));
    assert(peso == 1.5);
    for (i = 10U; i < 290U; i += 2U) {
        assert(grafo_obter_peso(grafo, i, i + 1U, &peso));
        assert(peso == (double)i / 4.0);
    }
    grafo_destruir(grafo);
}

static void teste_memoria_cresce_com_a_matriz_de_pesos(void)
{
    /* A matriz de pesos so e alocada quando aparece a primeira aresta com
     * peso: um grafo sem peso nao paga ordem x ordem doubles. */
    EstimativaMemoriaGrafo sem_peso, com_peso;
    Grafo *a = criar_com_antenas(GRAFO_MATRIZ_ADJACENCIA, 4U);
    Grafo *b = criar_com_antenas(GRAFO_MATRIZ_ADJACENCIA, 4U);

    assert(grafo_adicionar_aresta(a, 0U, 1U));
    assert(grafo_adicionar_aresta_ponderada(b, 0U, 1U, 5.0));
    assert(grafo_estimar_memoria(a, &sem_peso));
    assert(grafo_estimar_memoria(b, &com_peso));
    assert(com_peso.memoria_matriz_total_bytes >
           sem_peso.memoria_matriz_total_bytes);

    grafo_destruir(a);
    grafo_destruir(b);
}

int main(void)
{
    teste_inserir_e_consultar(GRAFO_ESTRUTURA_CONJUNTA);
    teste_inserir_e_consultar(GRAFO_LISTA_ADJACENCIA);
    teste_inserir_e_consultar(GRAFO_MATRIZ_ADJACENCIA);
    teste_vizinhos_trazem_peso();
    teste_percorrer_arestas();
    teste_pesos_invalidos();
    teste_compatibilidade_com_a_fase_um();
    teste_pesos_sobrevivem_a_expansao();
    teste_memoria_cresce_com_a_matriz_de_pesos();
    puts("Testes de arestas ponderadas passaram.");
    return 0;
}