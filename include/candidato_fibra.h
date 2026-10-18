#ifndef CANDIDATO_FIBRA_H
#define CANDIDATO_FIBRA_H

#include <stddef.h>

#include "grafo.h"

/* Parametros padrao decididos na issue #65 e registrados em
 * docs/MODELAGEM_PESOS_FASE2.md. k e o raio devem ser calibrados pela quantidade de componentes conexos
 * que produzem. */
#define CANDIDATO_FIBRA_K_PADRAO 4U
#define CANDIDATO_FIBRA_RAIO_PADRAO_KM 50.0

typedef struct {
    size_t k;              /* vizinhos mais proximos por antena; > 0 */
    double raio_maximo_km; /* distancia maxima de uma rota; > 0 */
} ParametrosCandidatoFibra;

typedef struct {
    size_t vertices;
    size_t arestas;
    size_t componentes;        /* vertices isolados contam um componente cada */
    size_t vertices_isolados;  /* antenas sem nenhuma rota candidata */
    double peso_total_km;      /* soma dos pesos; nao e a metragem da AGM */
    double maior_aresta_km;
} RelatorioCandidatoFibra;

/* Parametros padrao da issue #65: k = 4 e raio maximo de 50 km. */
ParametrosCandidatoFibra candidato_fibra_parametros_padrao(void);

/* Constroi o grafo candidato de cabeamento sobre as antenas ja carregadas.
 *
 * Criterio: cada antena recebe uma rota para cada uma de suas k antenas mais
 * proximas, desde que a distancia nao passe de raio_maximo_km. O peso da
 * aresta e a distancia geodesica em quilometros (grafo_distancia_km). A
 * elegibilidade nao considera operadora: fibra e infraestrutura fisica.
 *
 * As arestas sao nao direcionadas e sem repeticao. O grau de um vertice pode
 * passar de k, porque ele tambem e escolhido por antenas que o tenham entre
 * as k mais proximas delas; e por isso que o grafo contem ciclos, ao
 * contrario da floresta gerada pela Fase I.
 *
 * O grafo precisa ter vertices e nenhuma aresta: a funcao acrescenta rotas,
 * nao substitui as conexoes da Fase I. Retorna 0 sem alterar o grafo quando
 * os parametros forem invalidos ou ja existirem arestas, e 0 com o grafo
 * parcialmente construido apenas em falha de memoria.
 *
 * Custo: O(V^2) distancias. Com V = 1.000 sao 10^6 avaliacoes.
 */
int candidato_fibra_construir(Grafo *grafo, ParametrosCandidatoFibra parametros,
    RelatorioCandidatoFibra *relatorio);

#endif