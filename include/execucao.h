#ifndef EXECUCAO_H
#define EXECUCAO_H

#include "dataset.h"

typedef struct {
    const char *dataset;
    size_t limite;
    const char *saida;
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
} ResultadoExecucao;

/* Retorna 1 em sucesso, 0 para ajuda e -1 para argumentos invalidos. */
int execucao_ler_opcoes(int argc, char *argv[], OpcoesExecucao *opcoes);
void execucao_exibir_ajuda(void);

/* Acrescenta uma execucao ao CSV. Recusa arquivos com outro cabecalho.
 * Retorna 0 em erro, inclusive falhas ao gravar ou fechar o arquivo.
 */
int execucao_salvar(const OpcoesExecucao *opcoes, const ResultadoExecucao *resultado);

#endif
