#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grafo.h"

struct Grafo {
    Vertice *vertices;
    size_t quantidade_vertices;
    size_t capacidade_vertices;
    Aresta *arestas;
    size_t quantidade_arestas;
    size_t capacidade_arestas;
    ListaAdjacencia lista_adjacencia;
    MatrizAdjacencia matriz_adjacencia;
};

static int reservar_vertices(Grafo *grafo)
{
    size_t capacidade = grafo->capacidade_vertices == 0U ? 128U : grafo->capacidade_vertices * 2U;
    Vertice *vertices = realloc(grafo->vertices, capacidade * sizeof(*vertices));

    if (vertices == NULL) return 0;
    grafo->vertices = vertices;
    grafo->capacidade_vertices = capacidade;
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

static int expandir_adjacencia(Grafo *grafo)
{
    size_t antiga_ordem = grafo->matriz_adjacencia.ordem;
    size_t nova_ordem = antiga_ordem == 0U ? 128U : antiga_ordem * 2U;
    NoAdjacencia **listas = realloc(grafo->lista_adjacencia.listas,
                                   nova_ordem * sizeof(*listas));
    unsigned char *dados;
    size_t linha;

    if (listas == NULL) return 0;
    dados = calloc(nova_ordem * nova_ordem, sizeof(*dados));
    if (dados == NULL) {
        grafo->lista_adjacencia.listas = listas;
        return 0;
    }
    for (linha = 0U; linha < antiga_ordem; ++linha) {
        memcpy(&dados[linha * nova_ordem],
               &grafo->matriz_adjacencia.dados[linha * antiga_ordem],
               antiga_ordem * sizeof(*dados));
    }
    for (linha = antiga_ordem; linha < nova_ordem; ++linha) listas[linha] = NULL;
    free(grafo->matriz_adjacencia.dados);
    grafo->lista_adjacencia.listas = listas;
    grafo->lista_adjacencia.quantidade_vertices = nova_ordem;
    grafo->matriz_adjacencia.dados = dados;
    grafo->matriz_adjacencia.ordem = nova_ordem;
    return 1;
}

static void liberar_lista(NoAdjacencia *lista)
{
    while (lista != NULL) {
        NoAdjacencia *proximo = lista->proximo;
        free(lista);
        lista = proximo;
    }
}

Grafo *grafo_criar(void)
{
    return calloc(1U, sizeof(Grafo));
}

void grafo_destruir(Grafo *grafo)
{
    size_t indice;

    if (grafo == NULL) return;
    for (indice = 0U; indice < grafo->lista_adjacencia.quantidade_vertices; ++indice)
        liberar_lista(grafo->lista_adjacencia.listas[indice]);
    free(grafo->lista_adjacencia.listas);
    free(grafo->matriz_adjacencia.dados);
    free(grafo->vertices);
    free(grafo->arestas);
    free(grafo);
}

int grafo_adicionar_vertice(Grafo *grafo, Vertice vertice)
{
    if (grafo == NULL || !isfinite(vertice.coordenadas.latitude) ||
        !isfinite(vertice.coordenadas.longitude) || !isfinite(vertice.alcance_metros) ||
        vertice.coordenadas.latitude < -90.0 || vertice.coordenadas.latitude > 90.0 ||
        vertice.coordenadas.longitude < -180.0 || vertice.coordenadas.longitude > 180.0 ||
        vertice.alcance_metros < 0.0) return -1;
    if (grafo->quantidade_vertices == grafo->capacidade_vertices && !reservar_vertices(grafo))
        return -1;
    if (grafo->quantidade_vertices == grafo->matriz_adjacencia.ordem && !expandir_adjacencia(grafo))
        return -1;
    vertice.id = grafo->quantidade_vertices;
    grafo->vertices[grafo->quantidade_vertices] = vertice;
    ++grafo->quantidade_vertices;
    return (int)vertice.id;
}

int grafo_adicionar_antena(Grafo *grafo, unsigned int mcc, unsigned int net,
                           unsigned int area, unsigned int cell, double latitude,
                           double longitude, double alcance_metros)
{
    Vertice vertice = {0U, mcc, net, area, cell, {latitude, longitude}, alcance_metros};
    return grafo_adicionar_vertice(grafo, vertice);
}

int grafo_adicionar_aresta(Grafo *grafo, size_t origem, size_t destino)
{
    NoAdjacencia *origem_no;
    NoAdjacencia *destino_no;

    if (grafo == NULL || origem == destino || origem >= grafo->quantidade_vertices ||
        destino >= grafo->quantidade_vertices || grafo_sao_adjacentes(grafo, origem, destino))
        return 0;
    if (grafo->quantidade_arestas == grafo->capacidade_arestas && !reservar_arestas(grafo))
        return 0;
    origem_no = malloc(sizeof(*origem_no));
    destino_no = malloc(sizeof(*destino_no));
    if (origem_no == NULL || destino_no == NULL) {
        free(origem_no);
        free(destino_no);
        return 0;
    }
    origem_no->vertice = destino;
    origem_no->proximo = grafo->lista_adjacencia.listas[origem];
    destino_no->vertice = origem;
    destino_no->proximo = grafo->lista_adjacencia.listas[destino];
    grafo->lista_adjacencia.listas[origem] = origem_no;
    grafo->lista_adjacencia.listas[destino] = destino_no;
    grafo->matriz_adjacencia.dados[origem * grafo->matriz_adjacencia.ordem + destino] = 1U;
    grafo->matriz_adjacencia.dados[destino * grafo->matriz_adjacencia.ordem + origem] = 1U;
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
    if (arquivo == NULL) {
        if (erro != NULL && tamanho_erro > 0U)
            snprintf(erro, tamanho_erro, "Nao foi possivel abrir o CSV.");
        return 0U;
    }
    if (fgets(linha, sizeof(linha), arquivo) == NULL ||
        strncmp(linha, "radio,mcc,net,area,cell,lat,lon,range,samples", 43U) != 0) {
        if (erro != NULL && tamanho_erro > 0U)
            snprintf(erro, tamanho_erro, "Cabecalho CSV invalido.");
        fclose(arquivo);
        return 0U;
    }
    while ((max_antenas == 0U || carregadas < max_antenas) &&
           fgets(linha, sizeof(linha), arquivo) != NULL) {
        unsigned int mcc, net, area, cell, amostras;
        double latitude, longitude, alcance;
        if (sscanf(linha, "%15[^,],%u,%u,%u,%u,%lf,%lf,%lf,%u", radio, &mcc, &net, &area,
                   &cell, &latitude, &longitude, &alcance, &amostras) == 9 &&
            grafo_adicionar_antena(grafo, mcc, net, area, cell, latitude, longitude, alcance) >= 0)
            ++carregadas;
    }
    fclose(arquivo);
    return carregadas;
}

static double distancia_metros(const Vertice *a, const Vertice *b)
{
    const double radianos = 0.017453292519943295;
    const double raio_terra = 6371000.0;
    double dlat = (b->coordenadas.latitude - a->coordenadas.latitude) * radianos;
    double dlon = (b->coordenadas.longitude - a->coordenadas.longitude) * radianos;
    double seno_lat = sin(dlat / 2.0);
    double seno_lon = sin(dlon / 2.0);
    double x = seno_lat * seno_lat + cos(a->coordenadas.latitude * radianos) *
               cos(b->coordenadas.latitude * radianos) * seno_lon * seno_lon;
    return raio_terra * 2.0 * atan2(sqrt(x), sqrt(1.0 - x));
}

size_t grafo_construir_conexoes(Grafo *grafo)
{
    size_t *proxima;
    size_t i, j;
    double *menor;

    if (grafo == NULL || grafo->quantidade_vertices < 2U) return 0U;
    for (i = 0U; i < grafo->lista_adjacencia.quantidade_vertices; ++i) {
        liberar_lista(grafo->lista_adjacencia.listas[i]);
        grafo->lista_adjacencia.listas[i] = NULL;
    }
    memset(grafo->matriz_adjacencia.dados, 0,
           grafo->matriz_adjacencia.ordem * grafo->matriz_adjacencia.ordem);
    grafo->quantidade_arestas = 0U;
    proxima = malloc(grafo->quantidade_vertices * sizeof(*proxima));
    menor = malloc(grafo->quantidade_vertices * sizeof(*menor));
    if (proxima == NULL || menor == NULL) {
        free(proxima);
        free(menor);
        return 0U;
    }
    for (i = 0U; i < grafo->quantidade_vertices; ++i) {
        proxima[i] = grafo->quantidade_vertices;
        menor[i] = INFINITY;
    }
    for (i = 0U; i < grafo->quantidade_vertices; ++i) {
        for (j = i + 1U; j < grafo->quantidade_vertices; ++j) {
            Vertice *a = &grafo->vertices[i];
            Vertice *b = &grafo->vertices[j];
            double distancia;
            if (a->mcc != b->mcc || a->net != b->net || a->area != b->area) continue;
            distancia = distancia_metros(a, b);
            if (distancia > a->alcance_metros + b->alcance_metros) continue;
            if (distancia < menor[i]) { menor[i] = distancia; proxima[i] = j; }
            if (distancia < menor[j]) { menor[j] = distancia; proxima[j] = i; }
        }
    }
    for (i = 0U; i < grafo->quantidade_vertices; ++i) {
        j = proxima[i];
        if (j < grafo->quantidade_vertices && (proxima[j] != i || i < j))
            grafo_adicionar_aresta(grafo, i, j);
    }
    free(proxima);
    free(menor);
    return grafo->quantidade_arestas;
}

size_t grafo_quantidade_vertices(const Grafo *grafo)
{
    return grafo == NULL ? 0U : grafo->quantidade_vertices;
}

size_t grafo_quantidade_arestas(const Grafo *grafo)
{
    return grafo == NULL ? 0U : grafo->quantidade_arestas;
}

int grafo_sao_adjacentes(const Grafo *grafo, size_t origem, size_t destino)
{
    if (grafo == NULL || origem >= grafo->matriz_adjacencia.ordem ||
        destino >= grafo->matriz_adjacencia.ordem) return 0;
    return grafo->matriz_adjacencia.dados[origem * grafo->matriz_adjacencia.ordem + destino] != 0U;
}

static double orientacao(const Vertice *a, const Vertice *b, const Vertice *c)
{
    return (b->coordenadas.longitude - a->coordenadas.longitude) *
           (c->coordenadas.latitude - a->coordenadas.latitude) -
           (b->coordenadas.latitude - a->coordenadas.latitude) *
           (c->coordenadas.longitude - a->coordenadas.longitude);
}

int grafo_possui_cruzamentos(const Grafo *grafo)
{
    size_t i, j;

    if (grafo == NULL) return 0;
    for (i = 0U; i < grafo->quantidade_arestas; ++i) {
        for (j = i + 1U; j < grafo->quantidade_arestas; ++j) {
            Aresta a = grafo->arestas[i];
            Aresta b = grafo->arestas[j];
            double o1, o2, o3, o4;
            if (a.origem == b.origem || a.origem == b.destino ||
                a.destino == b.origem || a.destino == b.destino) continue;
            o1 = orientacao(&grafo->vertices[a.origem], &grafo->vertices[a.destino], &grafo->vertices[b.origem]);
            o2 = orientacao(&grafo->vertices[a.origem], &grafo->vertices[a.destino], &grafo->vertices[b.destino]);
            o3 = orientacao(&grafo->vertices[b.origem], &grafo->vertices[b.destino], &grafo->vertices[a.origem]);
            o4 = orientacao(&grafo->vertices[b.origem], &grafo->vertices[b.destino], &grafo->vertices[a.destino]);
            if (((o1 > 0.0 && o2 < 0.0) || (o1 < 0.0 && o2 > 0.0)) &&
                ((o3 > 0.0 && o4 < 0.0) || (o3 < 0.0 && o4 > 0.0))) return 1;
        }
    }
    return 0;
}
