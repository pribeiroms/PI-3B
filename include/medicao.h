#ifndef MEDICAO_H
#define MEDICAO_H

/* Medicao de desempenho dos algoritmos da Fase II (issue #86, RF03).
 *
 * Duas pecas: o cronometro de tempo decorrido e o registro de uma execucao
 * (algoritmo, estrutura, tamanho, tempo, memoria e resultado). A gravacao do
 * registro em CSV entra na etapa seguinte da mesma issue.
 */

#include <stddef.h>

#include "grafo.h"

/* Cronometro de tempo decorrido (relogio de parede, monotonico).
 *
 * No Windows usa QueryPerformanceCounter; nos demais sistemas, clock_gettime
 * com CLOCK_MONOTONIC. Nos dois a resolucao e bem menor que 1 ms. Ja o
 * clock() tem resolucao de 1 ms (CLOCKS_PER_SEC = 1000 nesta CRT), o que fazia
 * etapas rapidas aparecerem como 0.000 ms nos registros da Fase I.
 *
 * Mede tempo decorrido, nao tempo de CPU: os algoritmos da Fase II sao
 * sequenciais e nao esperam por I/O, entao os dois valores ficam proximos,
 * mas outros processos da maquina podem inflar a medida.
 */
typedef struct {
    unsigned long long marca; /* leitura bruta do relogio; unidade interna */
    int valido;               /* 0 se o relogio nao estava disponivel */
} Cronometro;

/* Marca o instante inicial. Se o relogio do sistema falhar, o cronometro fica
 * invalido e cronometro_decorrido_ms devolve -1.0. Aceita NULL sem fazer nada.
 */
void cronometro_iniciar(Cronometro *cronometro);

/* Milissegundos, com fracao, desde cronometro_iniciar. Pode ser chamada varias
 * vezes sobre o mesmo cronometro. Devolve -1.0 se o cronometro for NULL, se nao
 * tiver sido iniciado com sucesso ou se o relogio falhar: a mesma convencao de
 * "indisponivel" dos tempos da Fase I. O cronometro precisa ter passado por
 * cronometro_iniciar antes.
 */
double cronometro_decorrido_ms(const Cronometro *cronometro);

/* Como a execucao terminou. coloracao_exata pode parar por limite de tempo ou
 * de tamanho; sem este campo, uma linha interrompida pareceria um resultado. */
typedef enum {
    MEDICAO_STATUS_OK,
    MEDICAO_STATUS_LIMITE_TEMPO,
    MEDICAO_STATUS_LIMITE_TAMANHO,
    MEDICAO_STATUS_ERRO
} StatusMedicao;

/* Uma execucao de um algoritmo. A funcao medicao_preencher_do_grafo preenche
 * o que vem do grafo; quem executa o algoritmo preenche o resto (tempo, memoria
 * auxiliar, resultado e status) diretamente nos campos. */
typedef struct {
    const char *algoritmo;        /* ex.: "kruskal"; o texto precisa continuar
                                   * valido ate a gravacao (use literais) */
    EstruturaGrafo estrutura;     /* lista ou matriz */
    size_t limite_n;              /* N pedido ao carregar o dataset (0 = todos) */
    size_t vertices;
    size_t arestas;
    double tempo_ms;              /* de cronometro_decorrido_ms */
    size_t memoria_grafo_bytes;   /* estimativa da estrutura em uso */
    size_t memoria_auxiliar_bytes;/* buffers do algoritmo; 0 se nao informado */
    int tem_custo_agm;            /* 1 se custo_agm_km vale */
    double custo_agm_km;          /* soma dos pesos da AGM, em km */
    int tem_numero_cores;         /* 1 se numero_cores vale */
    size_t numero_cores;          /* canais usados pela coloracao */
    StatusMedicao status;
} RegistroMedicao;

/* Preenche algoritmo, estrutura, limite_n, vertices, arestas e
 * memoria_grafo_bytes a partir do grafo. memoria_grafo_bytes e o total da
 * estrutura em uso (lista ou matriz), de grafo_estimar_memoria; os demais
 * campos ficam zerados e o status, MEDICAO_STATUS_OK.
 *
 * Retorna 1 em sucesso. Retorna 0, sem alterar o registro, se algum ponteiro
 * for NULL, se a estrutura for a conjunta (ela mantem as duas representacoes e
 * nao serve para comparar memoria) ou se a estimativa estiver indisponivel.
 */
int medicao_preencher_do_grafo(RegistroMedicao *registro, const char *algoritmo,
    const Grafo *grafo, size_t limite_n);

#endif