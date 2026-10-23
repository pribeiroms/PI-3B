#include <stdlib.h>
#include <string.h>

#include "interferencia.h"

/* Mesma operadora: mesmo codigo de pais (mcc), mesma rede (net) e mesma area
 * de localizacao (area/LAC). E o mesmo teste do laco da Fase I em
 * grafo_construir_conexoes_ex. */
static int mesma_operadora(const Vertice *a, const Vertice *b)
{
    return a->mcc == b->mcc && a->net == b->net && a->area == b->area;
}

/* Os alcances vem em metros no dataset OpenCelliD, enquanto a distancia do
 * projeto e em quilometros. A comparacao e feita em metros para nao dividir os
 * alcances, que sao os valores vindos do dado bruto. */
static int sinais_se_sobrepoem(const Vertice *a, const Vertice *b)
{
    double distancia_metros =
        grafo_distancia_km(a->coordenadas, b->coordenadas) * 1000.0;
    return distancia_metros <= a->alcance_metros + b->alcance_metros;
}

int interferencia_construir(Grafo *grafo, RelatorioInterferencia *relatorio)
{
    Vertice *antenas = NULL;
    size_t *graus = NULL;
    size_t vertices, i, j;
    int ok = 0;

    if (grafo == NULL || relatorio == NULL) return 0;
    if (grafo_quantidade_arestas(grafo) != 0U) return 0;

    memset(relatorio, 0, sizeof(*relatorio));
    vertices = grafo_quantidade_vertices(grafo);
    relatorio->vertices = vertices;
    /* Sem par possivel nao ha sobreposicao: a unica antena, se houver, fica
     * isolada e o grau maximo e zero. */
    relatorio->vertices_isolados = vertices;
    if (vertices < 2U) return 1;

    /* Os dados das antenas sao copiados uma vez: o laco de pares os le V
     * vezes, e grafo_obter_vertice custa uma chamada a cada leitura. */
    antenas = malloc(vertices * sizeof(*antenas));
    graus = calloc(vertices, sizeof(*graus));
    if (antenas == NULL || graus == NULL) goto fim;

    for (i = 0U; i < vertices; ++i)
        if (!grafo_obter_vertice(grafo, i, &antenas[i])) goto fim;

    /* j > i: cada par e visitado uma unica vez, o que ja garante arestas sem
     * repeticao e sem laco, dispensando a consulta de adjacencia. */
    for (i = 0U; i < vertices; ++i) {
        for (j = i + 1U; j < vertices; ++j) {
            if (!mesma_operadora(&antenas[i], &antenas[j])) continue;
            if (!sinais_se_sobrepoem(&antenas[i], &antenas[j])) continue;
            if (!grafo_adicionar_aresta(grafo, i, j)) goto fim;
            ++graus[i];
            ++graus[j];
        }
    }

    /* Os graus sao contados na insercao, e nao percorrendo a estrutura depois:
     * assim o resultado e o mesmo na lista e na matriz de adjacencia. */
    relatorio->arestas = grafo_quantidade_arestas(grafo);
    relatorio->vertices_isolados = 0U;
    for (i = 0U; i < vertices; ++i) {
        if (graus[i] > relatorio->grau_maximo) {
            relatorio->grau_maximo = graus[i];
            relatorio->vertice_grau_maximo = i;
        }
        if (graus[i] == 0U) ++relatorio->vertices_isolados;
    }
    ok = 1;

fim:
    free(antenas);
    free(graus);
    return ok;
}