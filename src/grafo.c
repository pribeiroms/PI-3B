#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grafo.h"

struct Grafo {
    EstruturaGrafo estrutura;
    Vertice *vertices;
    size_t quantidade_vertices;
    size_t capacidade_vertices;
    Aresta *arestas;
    size_t quantidade_arestas;
    size_t capacidade_arestas;
    size_t capacidade_listas;
    ListaAdjacencia lista_adjacencia;
    MatrizAdjacencia matriz_adjacencia;
};

static int tamanho_array(size_t quantidade, size_t tamanho, size_t *bytes)
{
    if (tamanho != 0U && quantidade > SIZE_MAX / tamanho) return 0;
    *bytes = quantidade * tamanho;
    return 1;
}

static int dobrar_capacidade(size_t atual, size_t inicial, size_t *nova)
{
    if (atual == 0U) { *nova = inicial; return 1; }
    if (atual > SIZE_MAX / 2U) return 0;
    *nova = atual * 2U;
    return 1;
}

static int reservar_vertices(Grafo *grafo)
{
    size_t capacidade, bytes;
    Vertice *vertices;

    if (!dobrar_capacidade(grafo->capacidade_vertices, 128U, &capacidade) ||
        !tamanho_array(capacidade, sizeof(*vertices), &bytes)) return 0;
    vertices = realloc(grafo->vertices, bytes);

    if (vertices == NULL) return 0;
    grafo->vertices = vertices;
    grafo->capacidade_vertices = capacidade;
    return 1;
}

static int reservar_arestas(Grafo *grafo)
{
    size_t capacidade, bytes;
    Aresta *arestas;

    if (!dobrar_capacidade(grafo->capacidade_arestas, 128U, &capacidade) ||
        !tamanho_array(capacidade, sizeof(*arestas), &bytes)) return 0;
    arestas = realloc(grafo->arestas, bytes);

    if (arestas == NULL) return 0;
    grafo->arestas = arestas;
    grafo->capacidade_arestas = capacidade;
    return 1;
}

static int expandir_adjacencia(Grafo *grafo)
{
    size_t antiga_ordem = grafo->capacidade_listas > grafo->matriz_adjacencia.ordem ?
        grafo->capacidade_listas : grafo->matriz_adjacencia.ordem;
    size_t nova_ordem, bytes;
    size_t linha;
    if (!dobrar_capacidade(antiga_ordem, 128U, &nova_ordem)) return 0;
    if (grafo->estrutura != GRAFO_MATRIZ_ADJACENCIA) {
        NoAdjacencia **listas;
        if (!tamanho_array(nova_ordem, sizeof(*listas), &bytes)) return 0;
        listas = realloc(grafo->lista_adjacencia.listas, bytes);
        if (listas == NULL) return 0;
        for (linha = grafo->capacidade_listas; linha < nova_ordem; ++linha)
            listas[linha] = NULL;
        grafo->lista_adjacencia.listas = listas;
        grafo->capacidade_listas = nova_ordem;
        grafo->lista_adjacencia.quantidade_vertices = nova_ordem;
    }
    if (grafo->estrutura != GRAFO_LISTA_ADJACENCIA) {
        size_t ordem_matriz = grafo->matriz_adjacencia.ordem;
        size_t celulas;
        unsigned char *dados;
        if (!tamanho_array(nova_ordem, nova_ordem, &celulas)) return 0;
        dados = calloc(celulas, sizeof(*dados));
        if (dados == NULL) return 0;
        for (linha = 0U; linha < ordem_matriz; ++linha) {
            memcpy(&dados[linha * nova_ordem],
                   &grafo->matriz_adjacencia.dados[linha * ordem_matriz],
                   ordem_matriz * sizeof(*dados));
        }
        free(grafo->matriz_adjacencia.dados);
        grafo->matriz_adjacencia.dados = dados;
        grafo->matriz_adjacencia.ordem = nova_ordem;
    }
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
    return grafo_criar_com_estrutura(GRAFO_ESTRUTURA_CONJUNTA);
}

Grafo *grafo_criar_com_estrutura(EstruturaGrafo estrutura)
{
    Grafo *grafo;
    if (estrutura != GRAFO_ESTRUTURA_CONJUNTA &&
        estrutura != GRAFO_LISTA_ADJACENCIA &&
        estrutura != GRAFO_MATRIZ_ADJACENCIA) return NULL;
    grafo = calloc(1U, sizeof(*grafo));
    if (grafo != NULL) grafo->estrutura = estrutura;
    return grafo;
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
    size_t capacidade_adjacencia;
    if (grafo == NULL || !isfinite(vertice.coordenadas.latitude) ||
        !isfinite(vertice.coordenadas.longitude) || !isfinite(vertice.alcance_metros) ||
        vertice.coordenadas.latitude < -90.0 || vertice.coordenadas.latitude > 90.0 ||
        vertice.coordenadas.longitude < -180.0 || vertice.coordenadas.longitude > 180.0 ||
        vertice.alcance_metros < 0.0) return -1;
    if (grafo->quantidade_vertices == grafo->capacidade_vertices && !reservar_vertices(grafo))
        return -1;
    if (grafo->estrutura == GRAFO_LISTA_ADJACENCIA)
        capacidade_adjacencia = grafo->capacidade_listas;
    else if (grafo->estrutura == GRAFO_MATRIZ_ADJACENCIA)
        capacidade_adjacencia = grafo->matriz_adjacencia.ordem;
    else
        capacidade_adjacencia = grafo->capacidade_listas < grafo->matriz_adjacencia.ordem ?
            grafo->capacidade_listas : grafo->matriz_adjacencia.ordem;
    if (grafo->quantidade_vertices == capacidade_adjacencia && !expandir_adjacencia(grafo))
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
	if (grafo_adicionar_antena_ex(grafo, mcc, net, area, cell, latitude,
		longitude, alcance_metros) != 1) return -1;
	return (int)(grafo->quantidade_vertices - 1U);
}

int grafo_adicionar_antena_ex(Grafo *grafo, unsigned int mcc, unsigned int net,
						      unsigned int area, unsigned int cell, double latitude,
						      double longitude, double alcance_metros)
{
    Vertice vertice = {0U, mcc, net, area, cell, {latitude, longitude}, alcance_metros};
    if (grafo == NULL || !isfinite(latitude) || !isfinite(longitude) ||
        !isfinite(alcance_metros) || latitude < -90.0 || latitude > 90.0 ||
        longitude < -180.0 || longitude > 180.0 || alcance_metros < 0.0) return 0;
    return grafo_adicionar_vertice(grafo, vertice) < 0 ? -1 : 1;
}

int grafo_adicionar_aresta(Grafo *grafo, size_t origem, size_t destino)
{
    NoAdjacencia *origem_no = NULL;
    NoAdjacencia *destino_no = NULL;

    if (grafo == NULL || origem == destino || origem >= grafo->quantidade_vertices ||
        destino >= grafo->quantidade_vertices || grafo_sao_adjacentes(grafo, origem, destino))
        return 0;
    if (grafo->quantidade_arestas == grafo->capacidade_arestas && !reservar_arestas(grafo))
        return 0;
    if (grafo->estrutura != GRAFO_MATRIZ_ADJACENCIA) {
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
    }
    if (grafo->estrutura != GRAFO_LISTA_ADJACENCIA) {
        grafo->matriz_adjacencia.dados[origem * grafo->matriz_adjacencia.ordem + destino] = 1U;
        grafo->matriz_adjacencia.dados[destino * grafo->matriz_adjacencia.ordem + origem] = 1U;
    }
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

int grafo_construir_conexoes_ex(Grafo *grafo, size_t *arestas_construidas)
{
    size_t *proxima;
    size_t i, j;
    double *menor;

    if (arestas_construidas != NULL) *arestas_construidas = 0U;
    if (grafo == NULL || arestas_construidas == NULL) return 0;
    if (grafo->quantidade_vertices < 2U) return 1;
    if (grafo->estrutura != GRAFO_MATRIZ_ADJACENCIA) {
        for (i = 0U; i < grafo->lista_adjacencia.quantidade_vertices; ++i) {
            liberar_lista(grafo->lista_adjacencia.listas[i]);
            grafo->lista_adjacencia.listas[i] = NULL;
        }
    }
    if (grafo->estrutura != GRAFO_LISTA_ADJACENCIA) {
        memset(grafo->matriz_adjacencia.dados, 0,
               grafo->matriz_adjacencia.ordem * grafo->matriz_adjacencia.ordem);
    }
    grafo->quantidade_arestas = 0U;
    {
        size_t bytes_proxima, bytes_menor;
        if (!tamanho_array(grafo->quantidade_vertices, sizeof(*proxima), &bytes_proxima) ||
            !tamanho_array(grafo->quantidade_vertices, sizeof(*menor), &bytes_menor))
            return 0;
        proxima = malloc(bytes_proxima);
        menor = malloc(bytes_menor);
    }
    if (proxima == NULL || menor == NULL) {
        free(proxima);
        free(menor);
        return 0;
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
            if (!grafo_adicionar_aresta(grafo, i, j)) {
                free(proxima);
                free(menor);
                return 0;
            }
    }
    free(proxima);
    free(menor);
    *arestas_construidas = grafo->quantidade_arestas;
    return 1;
}

size_t grafo_construir_conexoes(Grafo *grafo)
{
    size_t arestas = 0U;
    return grafo_construir_conexoes_ex(grafo, &arestas) ? arestas : 0U;
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
    const NoAdjacencia *vizinho;
    if (grafo == NULL || origem >= grafo->quantidade_vertices ||
        destino >= grafo->quantidade_vertices) return 0;
    if (grafo->estrutura != GRAFO_LISTA_ADJACENCIA)
        return grafo->matriz_adjacencia.dados[
            origem * grafo->matriz_adjacencia.ordem + destino] != 0U;
    for (vizinho = grafo->lista_adjacencia.listas[origem]; vizinho != NULL;
         vizinho = vizinho->proximo)
        if (vizinho->vertice == destino) return 1;
    return 0;
}

const NoAdjacencia *grafo_vizinhos(const Grafo *grafo, size_t vertice)
{
    if (grafo == NULL || grafo->estrutura == GRAFO_MATRIZ_ADJACENCIA ||
        vertice >= grafo->quantidade_vertices) return NULL;
    return grafo->lista_adjacencia.listas[vertice];
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

    ResultadoEuler grafo_verificar_euler( const Grafo *grafo)
    {
      size_t v, e;

    if (grafo == NULL) return EULER_NAO_APLICAVEL;
    v = grafo->quantidade_vertices;
    e = grafo->quantidade_arestas;

    /* A desigualdade so vale para V >= 3. Este teste vem ANTES da conta
     * 3V-6, pois size_t nao tem sinal e daria underflow com V = 1 ou 2. */
    if (v < 3U) return EULER_NAO_APLICAVEL;

    /* Condicao necessaria: E <= 3V - 6. Se violada, nao e planar. */
    /* Calcular 3(V-2) evita overflow na multiplicação intermediária. */
    if (v - 2U > SIZE_MAX / 3U) return EULER_INCONCLUSIVO;
    if (e > 3U * (v - 2U)) return EULER_NAO_PLANAR;

    /* Satisfeita, mas Euler sozinho NAO garante planaridade (ex.: K3,3). */
    return EULER_INCONCLUSIVO;
    }

const char *grafo_mensagem_euler(ResultadoEuler resultado)
    {
     switch(resultado){
      case EULER_NAO_APLICAVEL:
          return "A validacao de Euler nao se apliica";
      case EULER_NAO_PLANAR:
          return "A validacao de Euler concluir que não e planar"; 
      case EULER_INCONCLUSIVO:
          return "A validacao de Euler foi aceita, mas e inconclusiva ";
     }

    return "";
    }

static double orientacao_coordenadas(CoordenadaGeografica a, CoordenadaGeografica b,
    CoordenadaGeografica c)
{
    return (b.longitude - a.longitude) * (c.latitude - a.latitude) -
        (b.latitude - a.latitude) * (c.longitude - a.longitude);
}

static int ponto_no_segmento(CoordenadaGeografica p, CoordenadaGeografica q,
    CoordenadaGeografica r)
{
    double min_lon = p.longitude < r.longitude ? p.longitude : r.longitude;
    double max_lon = p.longitude > r.longitude ? p.longitude : r.longitude;
    double min_lat = p.latitude < r.latitude ? p.latitude : r.latitude;
    double max_lat = p.latitude > r.latitude ? p.latitude : r.latitude;
    return q.longitude >= min_lon && q.longitude <= max_lon &&
        q.latitude >= min_lat && q.latitude <= max_lat;
}

static int segmentos_se_cruzam(Segmento a, Segmento b)
{
    double o1 = orientacao_coordenadas(a.inicio, a.fim, b.inicio);
    double o2 = orientacao_coordenadas(a.inicio, a.fim, b.fim);
    double o3 = orientacao_coordenadas(b.inicio, b.fim, a.inicio);
    double o4 = orientacao_coordenadas(b.inicio, b.fim, a.fim);
    if (((o1 > 0.0 && o2 < 0.0) || (o1 < 0.0 && o2 > 0.0)) &&
        ((o3 > 0.0 && o4 < 0.0) || (o3 < 0.0 && o4 > 0.0))) return 1;
    if (o1 == 0.0 && ponto_no_segmento(a.inicio, b.inicio, a.fim)) return 1;
    if (o2 == 0.0 && ponto_no_segmento(a.inicio, b.fim, a.fim)) return 1;
    if (o3 == 0.0 && ponto_no_segmento(b.inicio, a.inicio, b.fim)) return 1;
    if (o4 == 0.0 && ponto_no_segmento(b.inicio, a.fim, b.fim)) return 1;
    return 0;
}

size_t grafo_detectar_cruzamentos(const Grafo *grafo, Cruzamento **cruzamentos)
{
    size_t capacidade = 0U, quantidade = 0U, i, j;
    Cruzamento *encontrados = NULL;
    if (cruzamentos != NULL) *cruzamentos = NULL;
    if (grafo == NULL) return 0U;
    for (i = 0U; i < grafo->quantidade_arestas; ++i) {
        Aresta a = grafo->arestas[i];
        Segmento sa = {grafo->vertices[a.origem].coordenadas,
            grafo->vertices[a.destino].coordenadas};
        for (j = i + 1U; j < grafo->quantidade_arestas; ++j) {
            Aresta b = grafo->arestas[j];
            Segmento sb;
            Cruzamento *realocado;
            if (a.origem == b.origem || a.origem == b.destino ||
                a.destino == b.origem || a.destino == b.destino) continue;
            sb.inicio = grafo->vertices[b.origem].coordenadas;
            sb.fim = grafo->vertices[b.destino].coordenadas;
            if (!segmentos_se_cruzam(sa, sb)) continue;
            if (cruzamentos == NULL) {
                ++quantidade;
                continue;
            }
            if (quantidade == capacidade) {
                size_t nova_capacidade, bytes;
                if (!dobrar_capacidade(capacidade, 8U, &nova_capacidade) ||
                    !tamanho_array(nova_capacidade, sizeof(*realocado), &bytes)) {
                    free(encontrados);
                    return 0U;
                }
                realocado = realloc(encontrados, bytes);
                if (realocado == NULL) {
                    free(encontrados);
                    return 0U;
                }
                encontrados = realocado;
                capacidade = nova_capacidade;
            }
            encontrados[quantidade].aresta_a = i;
            encontrados[quantidade].aresta_b = j;
            ++quantidade;
        }
    }
    if (cruzamentos != NULL) *cruzamentos = encontrados;
    else free(encontrados);
    return quantidade;
}

static int somar_bytes(size_t a, size_t b, size_t *resultado)
{
    if (a > SIZE_MAX - b) return 0;
    *resultado = a + b;
    return 1;
}

static int multiplicar_bytes(size_t quantidade, size_t tamanho, size_t *resultado)
{
    if (tamanho != 0U && quantidade > SIZE_MAX / tamanho) return 0;
    *resultado = quantidade * tamanho;
    return 1;
}

int grafo_estimar_memoria(const Grafo *grafo, EstimativaMemoriaGrafo *estimativa)
{
    size_t vertices_bytes, arestas_bytes, listas_bytes, nos_bytes;
    size_t celulas_matriz, matriz_bytes, comum, lista_total, matriz_total;
    if (grafo == NULL || estimativa == NULL) return 0;
    if (!multiplicar_bytes(grafo->capacidade_vertices, sizeof(*grafo->vertices),
            &vertices_bytes) ||
        !multiplicar_bytes(grafo->capacidade_arestas, sizeof(*grafo->arestas),
            &arestas_bytes) ||
        !multiplicar_bytes(grafo->capacidade_listas,
            sizeof(*grafo->lista_adjacencia.listas), &listas_bytes) ||
        !multiplicar_bytes(grafo->quantidade_arestas, 2U * sizeof(NoAdjacencia),
            &nos_bytes) ||
        !multiplicar_bytes(grafo->matriz_adjacencia.ordem,
            grafo->matriz_adjacencia.ordem, &celulas_matriz) ||
        !multiplicar_bytes(celulas_matriz, sizeof(*grafo->matriz_adjacencia.dados),
            &matriz_bytes) ||
        !somar_bytes(sizeof(*grafo), vertices_bytes, &comum) ||
        !somar_bytes(comum, arestas_bytes, &comum) ||
        !somar_bytes(comum, listas_bytes, &lista_total) ||
        !somar_bytes(lista_total, nos_bytes, &lista_total) ||
        !somar_bytes(comum, matriz_bytes, &matriz_total)) return 0;
    if (grafo->estrutura == GRAFO_MATRIZ_ADJACENCIA) lista_total = 0U;
    if (grafo->estrutura == GRAFO_LISTA_ADJACENCIA) matriz_total = 0U;
    estimativa->memoria_comum_bytes = comum;
    estimativa->memoria_lista_total_bytes = lista_total;
    estimativa->memoria_matriz_total_bytes = matriz_total;
    return 1;
}
