#ifndef ANALISE_PLANARIDADE_H
#define ANALISE_PLANARIDADE_H

#include <stdio.h>
#include "grafo.h"

/* Consolida resultados ja calculados, sem executar os algoritmos novamente.
 * quantidade_cruzamentos == NULL indica contagem ainda indisponivel (#16).
 * Quando fornecida, a contagem prevalece sobre possui_cruzamentos e deve
 * corresponder ao mesmo grafo e a mesma execucao da analise de Euler.
 */
void analise_planaridade_exibir(FILE *saida, size_t vertices, size_t arestas,
    ResultadoEuler euler, int possui_cruzamentos,
    const size_t *quantidade_cruzamentos);

#endif
