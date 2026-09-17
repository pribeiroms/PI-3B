#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grafo.h"

typedef struct {
    unsigned int mcc, net, area, cell;
    double latitude, longitude, alcance;
} Antena;

typedef struct { size_t origem, destino; } Aresta;

struct Grafo {
    Antena *antenas;
    size_t quantidade_antenas, capacidade_antenas;
    Aresta *arestas;
    size_t quantidade_arestas, capacidade_arestas;
};

static int reservar_antenas(Grafo *grafo)
{
    size_t capacidade = grafo->capacidade_antenas == 0U ? 128U : grafo->capacidade_antenas * 2U;
    Antena *antenas = realloc(grafo->antenas, capacidade * sizeof(*antenas));
    if (antenas == NULL) return 0;
    grafo->antenas = antenas;
    grafo->capacidade_antenas = capacidade;
    return 1;
}

static int reservar_arestas(Grafo *grafo)
{
    size_t capacidade = grafo->capacidade_arestas == 0U ? 128U : grafo->capacidade_arestas * 2U;
    Aresta *arestas = realloc(grafo->arestas, capacidade * sizeof(*arestas));
    if (arestas == NULL) return 0;
    grafo->arestas = arestas;
    grafo->capacidade_arestas = capacidade;
    return 1;
}

Grafo *grafo_criar(void) { return calloc(1U, sizeof(Grafo)); }

void grafo_destruir(Grafo *grafo)
{
    if (grafo != NULL) {
        free(grafo->antenas);
        free(grafo->arestas);
        free(grafo);
    }
}

int grafo_adicionar_antena(Grafo *grafo, unsigned int mcc, unsigned int net,
                           unsigned int area, unsigned int cell, double latitude,
                           double longitude, double alcance_metros)
{
    Antena *antena;
    if (grafo == NULL || !isfinite(latitude) || !isfinite(longitude) || !isfinite(alcance_metros) ||
        latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0 ||
        alcance_metros < 0.0) return -1;
    if (grafo->quantidade_antenas == grafo->capacidade_antenas && !reservar_antenas(grafo)) return -1;
    antena = &grafo->antenas[grafo->quantidade_antenas];
    antena->mcc = mcc; antena->net = net; antena->area = area; antena->cell = cell;
    antena->latitude = latitude; antena->longitude = longitude; antena->alcance = alcance_metros;
    ++grafo->quantidade_antenas;
    return (int)(grafo->quantidade_antenas - 1U);
}

int grafo_adicionar_aresta(Grafo *grafo, size_t origem, size_t destino)
{
    size_t i;
    if (grafo == NULL || origem == destino || origem >= grafo->quantidade_antenas || destino >= grafo->quantidade_antenas) return 0;
    for (i = 0U; i < grafo->quantidade_arestas; ++i)
        if ((grafo->arestas[i].origem == origem && grafo->arestas[i].destino == destino) ||
            (grafo->arestas[i].origem == destino && grafo->arestas[i].destino == origem)) return 1;
    if (grafo->quantidade_arestas == grafo->capacidade_arestas && !reservar_arestas(grafo)) return 0;
    grafo->arestas[grafo->quantidade_arestas].origem = origem;
    grafo->arestas[grafo->quantidade_arestas].destino = destino;
    ++grafo->quantidade_arestas;
    return 1;
}

size_t grafo_carregar_csv(Grafo *grafo, const char *caminho, size_t max_antenas,
                          char *erro, size_t tamanho_erro)
{
    char linha[256], radio[16];
    FILE *arquivo;
    size_t carregadas = 0U;
    if (erro != NULL && tamanho_erro > 0U) erro[0] = '\0';
    if (grafo == NULL || caminho == NULL) return 0U;
    arquivo = fopen(caminho, "r");
    if (arquivo == NULL) { if (erro != NULL) snprintf(erro, tamanho_erro, "Nao foi possivel abrir o CSV."); return 0U; }
    if (fgets(linha, sizeof(linha), arquivo) == NULL || strncmp(linha, "radio,mcc,net,area,cell,lat,lon,range,samples", 43U) != 0) {
        if (erro != NULL) snprintf(erro, tamanho_erro, "Cabecalho CSV invalido.");
        fclose(arquivo); return 0U;
    }
    while ((max_antenas == 0U || carregadas < max_antenas) && fgets(linha, sizeof(linha), arquivo) != NULL) {
        unsigned int mcc, net, area, cell, amostras;
        double latitude, longitude, alcance;
        if (sscanf(linha, "%15[^,],%u,%u,%u,%u,%lf,%lf,%lf,%u", radio, &mcc, &net, &area, &cell,
                   &latitude, &longitude, &alcance, &amostras) == 9 &&
            grafo_adicionar_antena(grafo, mcc, net, area, cell, latitude, longitude, alcance) >= 0) ++carregadas;
    }
    fclose(arquivo);
    return carregadas;
}

static double distancia_metros(const Antena *a, const Antena *b)
{
    const double radianos = 0.017453292519943295, raio_terra = 6371000.0;
    double dlat = (b->latitude - a->latitude) * radianos;
    double dlon = (b->longitude - a->longitude) * radianos;
    double x = sin(dlat / 2.0) * sin(dlat / 2.0) + cos(a->latitude * radianos) *
               cos(b->latitude * radianos) * sin(dlon / 2.0) * sin(dlon / 2.0);
    return raio_terra * 2.0 * atan2(sqrt(x), sqrt(1.0 - x));
}

size_t grafo_construir_conexoes(Grafo *grafo)
{
    size_t *proxima, i, j, inicio;
    double *menor;
    if (grafo == NULL || grafo->quantidade_antenas < 2U) return 0U;
    free(grafo->arestas); grafo->arestas = NULL; grafo->quantidade_arestas = 0U; grafo->capacidade_arestas = 0U;
    proxima = malloc(grafo->quantidade_antenas * sizeof(*proxima));
    menor = malloc(grafo->quantidade_antenas * sizeof(*menor));
    if (proxima == NULL || menor == NULL) { free(proxima); free(menor); return 0U; }
    for (i = 0U; i < grafo->quantidade_antenas; ++i) { proxima[i] = grafo->quantidade_antenas; menor[i] = INFINITY; }
    for (i = 0U; i < grafo->quantidade_antenas; ++i) for (j = i + 1U; j < grafo->quantidade_antenas; ++j) {
        Antena *a = &grafo->antenas[i], *b = &grafo->antenas[j];
        double distancia;
        if (a->mcc != b->mcc || a->net != b->net || a->area != b->area) continue;
        distancia = distancia_metros(a, b);
        if (distancia > a->alcance + b->alcance) continue;
        if (distancia < menor[i]) { menor[i] = distancia; proxima[i] = j; }
        if (distancia < menor[j]) { menor[j] = distancia; proxima[j] = i; }
    }
    inicio = grafo->quantidade_arestas;
    for (i = 0U; i < grafo->quantidade_antenas; ++i) {
        j = proxima[i];
        if (j < grafo->quantidade_antenas && (proxima[j] != i || i < j) && !grafo_adicionar_aresta(grafo, i, j)) break;
    }
    free(proxima); free(menor);
    return grafo->quantidade_arestas - inicio;
}

size_t grafo_quantidade_vertices(const Grafo *grafo) { return grafo == NULL ? 0U : grafo->quantidade_antenas; }
size_t grafo_quantidade_arestas(const Grafo *grafo) { return grafo == NULL ? 0U : grafo->quantidade_arestas; }

static double orientacao(const Antena *a, const Antena *b, const Antena *c)
{
    return (b->longitude - a->longitude) * (c->latitude - a->latitude) -
           (b->latitude - a->latitude) * (c->longitude - a->longitude);
}

int grafo_possui_cruzamentos(const Grafo *grafo)
{
    size_t i, j;
    if (grafo == NULL) return 0;
    for (i = 0U; i < grafo->quantidade_arestas; ++i) for (j = i + 1U; j < grafo->quantidade_arestas; ++j) {
        Aresta a = grafo->arestas[i], b = grafo->arestas[j];
        double o1, o2, o3, o4;
        if (a.origem == b.origem || a.origem == b.destino || a.destino == b.origem || a.destino == b.destino) continue;
        o1 = orientacao(&grafo->antenas[a.origem], &grafo->antenas[a.destino], &grafo->antenas[b.origem]);
        o2 = orientacao(&grafo->antenas[a.origem], &grafo->antenas[a.destino], &grafo->antenas[b.destino]);
        o3 = orientacao(&grafo->antenas[b.origem], &grafo->antenas[b.destino], &grafo->antenas[a.origem]);
        o4 = orientacao(&grafo->antenas[b.origem], &grafo->antenas[b.destino], &grafo->antenas[a.destino]);
        if (((o1 > 0.0 && o2 < 0.0) || (o1 < 0.0 && o2 > 0.0)) &&
            ((o3 > 0.0 && o4 < 0.0) || (o3 < 0.0 && o4 > 0.0))) return 1;
    }
    return 0;
}
