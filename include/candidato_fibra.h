#ifndef CANDIDATO_FIBRA_H
#define CANDIDATO_FIBRA_H

#include <stddef.h>

#include "grafo.h"

/* Parametros padrao registrados em docs/MODELAGEM_PESOS_FASE2.md. k e o raio
 * sao calibrados pela quantidade de componentes conexos que produzem.
 *
 * k = 6 foi adotado na issue #71: sobre os subconjuntos densos da Fase II ele
 * produz um grafo CONEXO, e o Kruskal devolve uma arvore geradora minima unica
 * em vez de uma floresta. Com k = 4, que era o padrao da #65, restavam 5
 * componentes. A calibracao original, feita sobre as 1.000 primeiras linhas do
 * CSV, continua registrada no documento.
 *
 * O raio maximo nao tem efeito sobre os subconjuntos densos, que cabem inteiros
 * em poucos quilometros; ele permanece como salvaguarda para datasets
 * geograficamente espalhados, onde evita rotas irreais entre regioes distantes.
 */
#define CANDIDATO_FIBRA_K_PADRAO 6U
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