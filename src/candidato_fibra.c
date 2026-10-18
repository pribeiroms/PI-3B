#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "candidato_fibra.h"

ParametrosCandidatoFibra candidato_fibra_parametros_padrao(void)
{
    ParametrosCandidatoFibra parametros;
    parametros.k = CANDIDATO_FIBRA_K_PADRAO;
    parametros.raio_maximo_km = CANDIDATO_FIBRA_RAIO_PADRAO_KM;
    return parametros;
}

/* Guarda os k vizinhos mais proximos de uma antena, em ordem crescente de
 * distancia. Cabe em k posicoes: a cada candidato melhor que o pior da lista,
 * o pior e descartado. */
typedef struct {
    size_t *vertices;
    double *distancias;
    size_t capacidade;
    size_t quantidade;
} VizinhosMaisProximos;

static void vizinhos_reiniciar(VizinhosMaisProximos *vizinhos)
{
    vizinhos->quantidade = 0U;
}

static void vizinhos_oferecer(VizinhosMaisProximos *vizinhos, size_t vertice,
                              double distancia)
{
    size_t posicao;

    if (vizinhos->quantidade < vizinhos->capacidade) {
        posicao = vizinhos->quantidade++;
    } else if (distancia < vizinhos->distancias[vizinhos->capacidade - 1U]) {
        posicao = vizinhos->capacidade - 1U;
    } else {
        return;
    }
    while (posicao > 0U && vizinhos->distancias[posicao - 1U] > distancia) {
        vizinhos->distancias[posicao] = vizinhos->distancias[posicao - 1U];
        vizinhos->vertices[posicao] = vizinhos->vertices[posicao - 1U];
        --posicao;
    }
    vizinhos->distancias[posicao] = distancia;
    vizinhos->vertices[posicao] = vertice;
}

/* Marca, percorrendo o vetor de arestas, quais vertices receberam alguma
 * rota. Funciona em qualquer estrutura, porque o vetor de arestas existe nas
 * tres. */
static size_t contar_isolados(const Grafo *grafo, size_t vertices,
                              unsigned char *tem_rota)
{
    size_t indice, isolados = 0U;
    size_t origem, destino;

    memset(tem_rota, 0, vertices);
    for (indice = 0U; indice < grafo_quantidade_arestas(grafo); ++indice)
        if (grafo_obter_aresta_ponderada(grafo, indice, &origem, &destino, NULL)) {
            tem_rota[origem] = 1U;
            tem_rota[destino] = 1U;
        }
    for (indice = 0U; indice < vertices; ++indice)
        if (tem_rota[indice] == 0U) ++isolados;
    return isolados;
}

int candidato_fibra_construir(Grafo *grafo, ParametrosCandidatoFibra parametros,
                              RelatorioCandidatoFibra *relatorio)
{
    CoordenadaGeografica *coordenadas = NULL;
    VizinhosMaisProximos vizinhos;
    unsigned char *tem_rota = NULL;
    size_t vertices, i, j;
    int ok = 0;

    if (grafo == NULL || relatorio == NULL) return 0;
    if (parametros.k == 0U || !isfinite(parametros.raio_maximo_km) ||
        parametros.raio_maximo_km <= 0.0) return 0;
    if (grafo_quantidade_arestas(grafo) != 0U) return 0;

    memset(relatorio, 0, sizeof(*relatorio));
    vertices = grafo_quantidade_vertices(grafo);
    relatorio->vertices = vertices;
    relatorio->componentes = vertices;
    relatorio->vertices_isolados = vertices;
    if (vertices < 2U) return 1;

    vizinhos.capacidade = parametros.k < vertices - 1U ? parametros.k
        : vertices - 1U;
    vizinhos.quantidade = 0U;
    vizinhos.vertices = malloc(vizinhos.capacidade * sizeof(*vizinhos.vertices));
    vizinhos.distancias = malloc(vizinhos.capacidade *
        sizeof(*vizinhos.distancias));
    coordenadas = malloc(vertices * sizeof(*coordenadas));
    tem_rota = malloc(vertices);
    if (vizinhos.vertices == NULL || vizinhos.distancias == NULL ||
        coordenadas == NULL || tem_rota == NULL) goto fim;

    /* As coordenadas sao copiadas uma vez: o laco de pares as le V vezes. */
    for (i = 0U; i < vertices; ++i) {
        Vertice vertice;
        if (!grafo_obter_vertice(grafo, i, &vertice)) goto fim;
        coordenadas[i] = vertice.coordenadas;
    }

    for (i = 0U; i < vertices; ++i) {
        vizinhos_reiniciar(&vizinhos);
        for (j = 0U; j < vertices; ++j) {
            double distancia;
            if (j == i) continue;
            distancia = grafo_distancia_km(coordenadas[i], coordenadas[j]);
            if (distancia > parametros.raio_maximo_km) continue;
            vizinhos_oferecer(&vizinhos, j, distancia);
        }
        for (j = 0U; j < vizinhos.quantidade; ++j) {
            size_t destino = vizinhos.vertices[j];
            double distancia = vizinhos.distancias[j];
            /* A rota pode ja existir: i pode estar entre os k mais proximos
             * de destino e vice-versa. */
            if (grafo_sao_adjacentes(grafo, i, destino)) continue;
            if (!grafo_adicionar_aresta_ponderada(grafo, i, destino, distancia))
                goto fim;
            if (distancia > relatorio->maior_aresta_km)
                relatorio->maior_aresta_km = distancia;
        }
    }

    relatorio->arestas = grafo_quantidade_arestas(grafo);
    relatorio->peso_total_km = grafo_peso_total(grafo);
    relatorio->vertices_isolados = contar_isolados(grafo, vertices, tem_rota);
    if (!grafo_contar_componentes(grafo, &relatorio->componentes)) goto fim;
    ok = 1;

fim:
    free(vizinhos.vertices);
    free(vizinhos.distancias);
    free(coordenadas);
    free(tem_rota);
    return ok;
}