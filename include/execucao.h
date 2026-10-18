#ifndef EXECUCAO_H
#define EXECUCAO_H

#include "candidato_fibra.h"
#include "dataset.h"
#include "interferencia.h"

/* MODO_FASE1 executa a analise de planaridade da Fase I e grava no CSV de
 * resultados. MODO_CANDIDATO constroi o grafo candidato de cabeamento da
 * Fase II (issue #68) e apenas relata vertices, arestas e componentes: serve
 * para calibrar k e raio, e nao grava no CSV, que tem colunas da Fase I.
 * MODO_INTERFERENCIA constroi o grafo de interferencia da Fase II (issue #69)
 * e relata vertices, arestas e grau maximo; tambem nao grava no CSV. */
typedef enum {
    EXECUCAO_MODO_FASE1,
    EXECUCAO_MODO_CANDIDATO,
    EXECUCAO_MODO_INTERFERENCIA
} ModoExecucao;

typedef struct {
    const char *dataset;
    size_t limite;
    const char *saida;
    const char *dot;
    const char *dot_mapa;
    EstruturaGrafo estrutura;
    ModoExecucao modo;
    ParametrosCandidatoFibra candidato;
} OpcoesExecucao;

typedef struct {
    RelatorioCarregamento carregamento;
    size_t vertices;
    size_t arestas;
    ResultadoEuler euler;
    int possui_cruzamentos;
    size_t quantidade_cruzamentos;
    EstimativaMemoriaGrafo memoria;
    int memoria_disponivel;
    double leitura_ms;
    double construcao_ms;
    double euler_ms;
    double cruzamentos_ms;
    double total_cpu_ms;
} ResultadoExecucao;

/* Retorna 1 em sucesso, 0 para ajuda e -1 para argumentos invalidos. */
int execucao_ler_opcoes(int argc, char *argv[], OpcoesExecucao *opcoes);
void execucao_exibir_ajuda(void);

/* Acrescenta uma execucao ao CSV. Recusa arquivos com outro cabecalho.
 * Retorna 0 em erro, inclusive falhas ao gravar ou fechar o arquivo.
 */
int execucao_salvar(const OpcoesExecucao *opcoes, const ResultadoExecucao *resultado);

#endif