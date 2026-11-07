#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "grafo.h"
#include "medicao.h"

#define AMOSTRAS 10000U

static void teste_leitura_nao_negativa_e_finita(void)
{
    Cronometro cronometro;
    double tempo;

    cronometro_iniciar(&cronometro);
    tempo = cronometro_decorrido_ms(&cronometro);
    assert(isfinite(tempo));
    assert(tempo >= 0.0);
}

static void teste_leituras_nao_decrescem(void)
{
    Cronometro cronometro;
    double anterior = 0.0;
    unsigned int i;

    cronometro_iniciar(&cronometro);
    for (i = 0U; i < AMOSTRAS; ++i) {
        double atual = cronometro_decorrido_ms(&cronometro);
        assert(atual >= anterior);
        anterior = atual;
    }
}

/* Consulta o cronometro em laco ate ele marcar 5 ms. O limite de iteracoes
 * impede que um relogio quebrado, que devolve -1.0 sempre, prenda o teste para
 * sempre. */
static void teste_mede_intervalo_de_milissegundos(void)
{
    Cronometro cronometro;
    unsigned long long i;
    double tempo = 0.0;

    cronometro_iniciar(&cronometro);
    for (i = 0ULL; i < 50000000ULL && tempo < 5.0; ++i)
        tempo = cronometro_decorrido_ms(&cronometro);
    assert(tempo >= 5.0);
}

/* O motivo de existir do cronometro: distinguir instantes separados por menos
 * de 1 ms, onde clock() devolveria 0.000. Usa o menor passo positivo entre
 * leituras seguidas, que nao e afetado por uma preempcao ocasional. */
static void teste_resolucao_menor_que_um_milissegundo(void)
{
    Cronometro cronometro;
    double anterior;
    double menor_passo = 1.0e9;
    unsigned int i;

    cronometro_iniciar(&cronometro);
    anterior = cronometro_decorrido_ms(&cronometro);
    for (i = 0U; i < AMOSTRAS; ++i) {
        double atual = cronometro_decorrido_ms(&cronometro);
        double passo = atual - anterior;
        if (passo > 0.0 && passo < menor_passo) menor_passo = passo;
        anterior = atual;
    }
    assert(menor_passo < 1.0);
}

static void teste_entradas_invalidas(void)
{
    Cronometro invalido;

    cronometro_iniciar(NULL);
    assert(cronometro_decorrido_ms(NULL) == -1.0);
    invalido.marca = 0ULL;
    invalido.valido = 0;
    assert(cronometro_decorrido_ms(&invalido) == -1.0);
}

static Grafo *criar_grafo_pequeno(EstruturaGrafo estrutura)
{
    Grafo *grafo = grafo_criar_com_estrutura(estrutura);

    assert(grafo != NULL);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, 1ULL, -23.00, -46.00, 1000.0) == 0);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, 2ULL, -23.01, -46.00, 1000.0) == 1);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 10U, 3ULL, -23.02, -46.00, 1000.0) == 2);
    assert(grafo_adicionar_aresta_ponderada(grafo, 0U, 1U, 1.1) == 1);
    assert(grafo_adicionar_aresta_ponderada(grafo, 1U, 2U, 1.1) == 1);
    return grafo;
}

static void teste_registro_da_lista(void)
{
    Grafo *grafo = criar_grafo_pequeno(GRAFO_LISTA_ADJACENCIA);
    EstimativaMemoriaGrafo memoria;
    RegistroMedicao registro;

    assert(grafo_estimar_memoria(grafo, &memoria) == 1);
    assert(medicao_preencher_do_grafo(&registro, "kruskal", grafo, 1000U) == 1);
    assert(strcmp(registro.algoritmo, "kruskal") == 0);
    assert(registro.estrutura == GRAFO_LISTA_ADJACENCIA);
    assert(registro.limite_n == 1000U);
    assert(registro.vertices == 3U);
    assert(registro.arestas == 2U);
    assert(registro.memoria_grafo_bytes == memoria.memoria_lista_total_bytes);
    assert(registro.memoria_grafo_bytes > 0U);
    assert(registro.memoria_auxiliar_bytes == 0U);
    assert(registro.tem_custo_agm == 0);
    assert(registro.tem_numero_cores == 0);
    assert(registro.status == MEDICAO_STATUS_OK);
    grafo_destruir(grafo);
}

static void teste_registro_da_matriz(void)
{
    Grafo *grafo = criar_grafo_pequeno(GRAFO_MATRIZ_ADJACENCIA);
    EstimativaMemoriaGrafo memoria;
    RegistroMedicao registro;

    assert(grafo_estimar_memoria(grafo, &memoria) == 1);
    assert(medicao_preencher_do_grafo(&registro, "coloracao_gulosa", grafo, 0U) == 1);
    assert(registro.estrutura == GRAFO_MATRIZ_ADJACENCIA);
    assert(registro.limite_n == 0U);
    assert(registro.vertices == 3U);
    assert(registro.arestas == 2U);
    assert(registro.memoria_grafo_bytes == memoria.memoria_matriz_total_bytes);
    assert(registro.memoria_grafo_bytes > 0U);
    grafo_destruir(grafo);
}

static void teste_recusa_conjunta_e_entradas_invalidas(void)
{
    Grafo *conjunta = criar_grafo_pequeno(GRAFO_ESTRUTURA_CONJUNTA);
    Grafo *lista = criar_grafo_pequeno(GRAFO_LISTA_ADJACENCIA);
    RegistroMedicao registro;

    memset(&registro, 0, sizeof(registro));
    registro.vertices = 42U;
    assert(medicao_preencher_do_grafo(&registro, "kruskal", conjunta, 100U) == 0);
    assert(medicao_preencher_do_grafo(NULL, "kruskal", lista, 100U) == 0);
    assert(medicao_preencher_do_grafo(&registro, NULL, lista, 100U) == 0);
    assert(medicao_preencher_do_grafo(&registro, "kruskal", NULL, 100U) == 0);
    assert(registro.vertices == 42U); /* recusas nao alteram o registro */
    grafo_destruir(conjunta);
    grafo_destruir(lista);
}

int main(void)
{
    teste_leitura_nao_negativa_e_finita();
    teste_leituras_nao_decrescem();
    teste_mede_intervalo_de_milissegundos();
    teste_resolucao_menor_que_um_milissegundo();
    teste_entradas_invalidas();
    teste_registro_da_lista();
    teste_registro_da_matriz();
    teste_recusa_conjunta_e_entradas_invalidas();
    puts("Testes do cronometro passaram.");
    return 0;
}