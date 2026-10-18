#ifndef INTERFERENCIA_H
#define INTERFERENCIA_H

#include <stddef.h>

#include "grafo.h"

typedef struct {
    size_t vertices;
    size_t arestas;
    size_t grau_maximo;           /* o Delta que limita a coloracao gulosa */
    size_t vertice_grau_maximo;   /* indice da antena de maior grau */
    size_t vertices_isolados;     /* antenas que nao interferem em ninguem */
} RelatorioInterferencia;

/* Constroi o grafo de interferencia entre as antenas ja carregadas.
 *
 * Criterio (issue #69, registrado em docs/MODELAGEM_PESOS_FASE2.md): duas
 * antenas sao vizinhas quando pertencem a mesma operadora -- mesmo mcc, mesmo
 * net e mesma area -- e a distancia entre elas nao passa da soma dos alcances
 * estimados:
 *
 *     d(u, v) <= alcance(u) + alcance(v)
 *
 * E o mesmo predicado de elegibilidade da Fase I em
 * grafo_construir_conexoes_ex. A diferenca e o que se faz com ele: a Fase I
 * guardava apenas a vizinha elegivel mais proxima de cada antena, o que gera
 * uma floresta; aqui todas as vizinhas elegiveis entram, porque interferencia
 * nao escolhe a mais proxima -- quem se sobrepoe disputa o canal.
 *
 * Torres de operadoras diferentes NAO interferem entre si: cada operadora
 * recebe sua propria faixa de espectro, logo o conflito de canal so existe
 * dentro da mesma operadora.
 *
 * O grafo e NAO PONDERADO: a coloracao de frequencias so pergunta se duas
 * torres se sobrepoem, nao o quanto. Por isso usa grafo_adicionar_aresta e
 * nao a versao ponderada, e as estruturas da Fase I bastam.
 *
 * As arestas sao nao direcionadas e sem repeticao: o laco de pares visita cada
 * par (i, j) uma unica vez, com j > i.
 *
 * O grafo precisa ter vertices e nenhuma aresta: a funcao recusa um grafo que
 * ja tenha arestas, para nao misturar o criterio da Fase I, o do grafo
 * candidato de cabeamento (#68) e o da interferencia no mesmo grafo. Retorna 0
 * sem alterar o grafo nesse caso e no de entrada invalida, e 0 com o grafo
 * parcialmente construido apenas em falha de memoria.
 *
 * O grau maximo relatado e o Delta do grafo. A coloracao gulosa (#79) usa no
 * maximo Delta + 1 cores, ou seja, Delta + 1 e o limite superior de canais de
 * radio necessarios -- e por isso que a #69 precisa informa-lo.
 *
 * Custo: O(V^2) distancias, como o laco da Fase I. Com V = 1.000 sao 10^6
 * avaliacoes.
 */
int interferencia_construir(Grafo *grafo, RelatorioInterferencia *relatorio);

#endif