#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exportar_dot.h"
#include "utf8io.h"
/* Lado do desenho, em polegadas. Todos os recortes usam o mesmo lado: o que
 * muda e a area geografica que cabe nele. */
#define LADO_MAPA 36.0

/* Recorte por percentil. As antenas do dataset se concentram em poucas
 * regioes; algumas poucas, muito distantes, esticariam o enquadramento e
 * deixariam o restante amontoado num canto. */
#define PERCENTIL_RECORTE 4.0
#define MARGEM_RECORTE 0.08

/* Modo foco: lado de cada painel e espaco entre eles, em polegadas. */
#define LADO_PAINEL 9.0
#define ESPACO_PAINEL 4.0

/* Margem em volta do cruzamento, como fracao do lado do painel. */
#define MARGEM_PAINEL 0.35

typedef struct {
    double oeste;
    double leste;
    double sul;
    double norte;
} Caixa;

typedef struct {
    unsigned char *arestas_cruzadas;
    unsigned char *vertices_cruzados;
    unsigned char *vertices_incluidos; /* NULL inclui todos os vertices. */
} Selecao;

static void selecao_liberar(Selecao *selecao)
{
    free(selecao->arestas_cruzadas);
    free(selecao->vertices_cruzados);
    free(selecao->vertices_incluidos);
    memset(selecao, 0, sizeof(*selecao));
}

static int vertice_incluido(const Selecao *selecao, size_t indice)
{
    return selecao->vertices_incluidos == NULL ||
        selecao->vertices_incluidos[indice];
}

/* --- Destaque dos cruzamentos ------------------------------------------- */

static void marcar_extremos(const Grafo *grafo, size_t indice_aresta,
    unsigned char *vertices_marcados, size_t total_vertices)
{
    size_t origem, destino;
    if (!grafo_obter_aresta(grafo, indice_aresta, &origem, &destino)) return;
    if (origem < total_vertices) vertices_marcados[origem] = 1U;
    if (destino < total_vertices) vertices_marcados[destino] = 1U;
}

/* Marca quais arestas participam de cruzamentos e quais vertices sao extremos
 * dessas arestas. O destaque dos vertices e necessario porque as arestas
 * cruzadas ligam antenas muito proximas: na escala do mapa inteiro elas
 * ficariam menores que um pixel. */
static int marcar_cruzamentos(const Grafo *grafo, const Cruzamento *cruzamentos,
    size_t quantidade_cruzamentos, Selecao *selecao)
{
    size_t total_arestas = grafo_quantidade_arestas(grafo);
    size_t total_vertices = grafo_quantidade_vertices(grafo);
    size_t i;

    selecao->vertices_cruzados = calloc(total_vertices,
        sizeof(*selecao->vertices_cruzados));
    if (selecao->vertices_cruzados == NULL) return 0;
    if (total_arestas == 0U) return 1;
    selecao->arestas_cruzadas = calloc(total_arestas,
        sizeof(*selecao->arestas_cruzadas));
    if (selecao->arestas_cruzadas == NULL) return 0;
    if (cruzamentos == NULL) return 1;
    for (i = 0U; i < quantidade_cruzamentos; ++i) {
        if (cruzamentos[i].aresta_a < total_arestas) {
            selecao->arestas_cruzadas[cruzamentos[i].aresta_a] = 1U;
            marcar_extremos(grafo, cruzamentos[i].aresta_a,
                selecao->vertices_cruzados, total_vertices);
        }
        if (cruzamentos[i].aresta_b < total_arestas) {
            selecao->arestas_cruzadas[cruzamentos[i].aresta_b] = 1U;
            marcar_extremos(grafo, cruzamentos[i].aresta_b,
                selecao->vertices_cruzados, total_vertices);
        }
    }
    return 1;
}

/* --- Enquadramento ------------------------------------------------------- */

static int comparar_double(const void *a, const void *b)
{
    double x = *(const double *)a;
    double y = *(const double *)b;
    return x < y ? -1 : (x > y ? 1 : 0);
}

/* Caixa que contem todas as antenas carregadas. */
static int caixa_total(const Grafo *grafo, Caixa *caixa)
{
    size_t total = grafo_quantidade_vertices(grafo);
    size_t i;
    Vertice vertice;

    if (total == 0U || !grafo_obter_vertice(grafo, 0U, &vertice)) return 0;
    caixa->oeste = caixa->leste = vertice.coordenadas.longitude;
    caixa->sul = caixa->norte = vertice.coordenadas.latitude;
    for (i = 1U; i < total; ++i) {
        if (!grafo_obter_vertice(grafo, i, &vertice)) return 0;
        if (vertice.coordenadas.longitude < caixa->oeste)
            caixa->oeste = vertice.coordenadas.longitude;
        if (vertice.coordenadas.longitude > caixa->leste)
            caixa->leste = vertice.coordenadas.longitude;
        if (vertice.coordenadas.latitude < caixa->sul)
            caixa->sul = vertice.coordenadas.latitude;
        if (vertice.coordenadas.latitude > caixa->norte)
            caixa->norte = vertice.coordenadas.latitude;
    }
    return 1;
}

/* Caixa da faixa central das coordenadas, sem os extremos distantes. */
static int caixa_central(const Grafo *grafo, Caixa *caixa)
{
    size_t total = grafo_quantidade_vertices(grafo);
    double *longitudes, *latitudes, margem;
    size_t i, menor, maior;

    if (total == 0U) return 0;
    longitudes = malloc(total * sizeof(*longitudes));
    latitudes = malloc(total * sizeof(*latitudes));
    if (longitudes == NULL || latitudes == NULL) {
        free(longitudes);
        free(latitudes);
        return 0;
    }
    for (i = 0U; i < total; ++i) {
        Vertice vertice;
        if (!grafo_obter_vertice(grafo, i, &vertice)) {
            free(longitudes);
            free(latitudes);
            return 0;
        }
        longitudes[i] = vertice.coordenadas.longitude;
        latitudes[i] = vertice.coordenadas.latitude;
    }
    qsort(longitudes, total, sizeof(*longitudes), comparar_double);
    qsort(latitudes, total, sizeof(*latitudes), comparar_double);
    menor = (size_t)((double)(total - 1U) * PERCENTIL_RECORTE / 100.0);
    maior = (size_t)((double)(total - 1U) *
        (100.0 - PERCENTIL_RECORTE) / 100.0);
    caixa->oeste = longitudes[menor];
    caixa->leste = longitudes[maior];
    caixa->sul = latitudes[menor];
    caixa->norte = latitudes[maior];
    free(longitudes);
    free(latitudes);
    margem = (caixa->leste - caixa->oeste) * MARGEM_RECORTE;
    caixa->oeste -= margem;
    caixa->leste += margem;
    margem = (caixa->norte - caixa->sul) * MARGEM_RECORTE;
    caixa->sul -= margem;
    caixa->norte += margem;
    return 1;
}

/* Marca os vertices que caem dentro da caixa e conta os que ficam de fora. */
static int selecionar_na_caixa(const Grafo *grafo, Selecao *selecao,
    const Caixa *caixa, size_t *fora)
{
    size_t total = grafo_quantidade_vertices(grafo);
    size_t i;

    selecao->vertices_incluidos = calloc(total,
        sizeof(*selecao->vertices_incluidos));
    if (selecao->vertices_incluidos == NULL) return 0;
    *fora = 0U;
    for (i = 0U; i < total; ++i) {
        Vertice vertice;
        if (!grafo_obter_vertice(grafo, i, &vertice)) return 0;
        if (vertice.coordenadas.longitude >= caixa->oeste &&
            vertice.coordenadas.longitude <= caixa->leste &&
            vertice.coordenadas.latitude >= caixa->sul &&
            vertice.coordenadas.latitude <= caixa->norte)
            selecao->vertices_incluidos[i] = 1U;
        else
            ++(*fora);
    }
    return 1;
}

/* --- Contagem ------------------------------------------------------------ */

static size_t contar_incluidos(const Grafo *grafo, const Selecao *selecao)
{
    size_t total = grafo_quantidade_vertices(grafo);
    size_t i, contagem = 0U;
    for (i = 0U; i < total; ++i)
        if (vertice_incluido(selecao, i)) ++contagem;
    return contagem;
}

static size_t contar_arestas_incluidas(const Grafo *grafo,
    const Selecao *selecao)
{
    size_t total = grafo_quantidade_arestas(grafo);
    size_t i, contagem = 0U;
    for (i = 0U; i < total; ++i) {
        size_t origem, destino;
        if (!grafo_obter_aresta(grafo, i, &origem, &destino)) continue;
        if (vertice_incluido(selecao, origem) &&
            vertice_incluido(selecao, destino)) ++contagem;
    }
    return contagem;
}

static size_t contar_cruzamentos_visiveis(const Grafo *grafo,
    const Selecao *selecao, const Cruzamento *cruzamentos, size_t quantidade)
{
    size_t total_arestas = grafo_quantidade_arestas(grafo);
    size_t i, contagem = 0U;

    if (cruzamentos == NULL) return 0U;
    for (i = 0U; i < quantidade; ++i) {
        size_t oa, da, ob, db;
        if (cruzamentos[i].aresta_a >= total_arestas ||
            cruzamentos[i].aresta_b >= total_arestas) continue;
        if (!grafo_obter_aresta(grafo, cruzamentos[i].aresta_a, &oa, &da) ||
            !grafo_obter_aresta(grafo, cruzamentos[i].aresta_b, &ob, &db))
            continue;
        if (vertice_incluido(selecao, oa) && vertice_incluido(selecao, da) &&
            vertice_incluido(selecao, ob) && vertice_incluido(selecao, db))
            ++contagem;
    }
    return contagem;
}

/* Diametro dos vertices, em polegadas.
 *
 * O tamanho vem do comprimento tipico das arestas exibidas, nao da quantidade
 * de vertices: o que torna um desenho legivel e o vertice ser bem menor que a
 * rota que sai dele. Com um terco da mediana, cabem tres diametros ao longo de
 * uma rota tipica, e as linhas continuam visiveis quando se amplia a imagem.
 * Sem arestas no recorte, usa uma fracao da area enquadrada.
 */
/* Na area completa as antenas distantes dominam o enquadramento e a mediana
 * das arestas fica pequena demais frente ao desenho: ali o tamanho vem da
 * quantidade de vertices, o que mantem o mapa visivel sem ampliacao. */
static double largura_por_quantidade(size_t exibidos)
{
    double largura;
    if (exibidos == 0U) return 0.40;
    largura = 6.0 / sqrt((double)exibidos);
    if (largura < 0.12) largura = 0.12;
    if (largura > 0.45) largura = 0.45;
    return largura;
}

static double largura_vertice(const Grafo *grafo, const Selecao *selecao,
    double escala, double lado_graus)
{
    size_t total = grafo_quantidade_arestas(grafo);
    double *comprimentos, mediana, largura;
    size_t i, quantidade = 0U;

    comprimentos = total > 0U ? malloc(total * sizeof(*comprimentos)) : NULL;
    if (comprimentos != NULL) {
        for (i = 0U; i < total; ++i) {
            size_t origem, destino;
            Vertice a, b;
            double dx, dy;
            if (!grafo_obter_aresta(grafo, i, &origem, &destino)) continue;
            if (!vertice_incluido(selecao, origem) ||
                !vertice_incluido(selecao, destino)) continue;
            if (!grafo_obter_vertice(grafo, origem, &a) ||
                !grafo_obter_vertice(grafo, destino, &b)) continue;
            dx = a.coordenadas.longitude - b.coordenadas.longitude;
            dy = a.coordenadas.latitude - b.coordenadas.latitude;
            comprimentos[quantidade++] = sqrt(dx * dx + dy * dy);
        }
    }
    if (quantidade > 0U) {
        qsort(comprimentos, quantidade, sizeof(*comprimentos), comparar_double);
        mediana = comprimentos[quantidade / 2U];
    } else {
        mediana = lado_graus / 60.0;
    }
    free(comprimentos);
    largura = mediana * escala / 2.5;
    if (largura < 0.020) largura = 0.020;
    if (largura > 0.200) largura = 0.200;
    return largura;
}

/* --- Escrita ------------------------------------------------------------- */

static void escrever_cabecalho_mapa(FILE *arquivo, const char *regiao,
    size_t vertices, size_t arestas, size_t cruzamentos, size_t fora,
    double largura)
{
    double fonte = largura * 28.0;
    if (fonte < 1.5) fonte = 1.5;
    if (fonte > 12.0) fonte = 12.0;
    fprintf(arquivo,
        "// Gerado por grafo.exe: posicoes geograficas reais.\n"
        "// Recorte: %s | Exibidos: %zu | Fora: %zu | Cruzamentos: %zu\n"
        "graph Mapa {\n"
        "  graph [layout=neato, overlap=true, splines=false, bgcolor=white,\n"
        "         labelloc=\"t\", fontsize=22, fontname=\"Helvetica\",\n"
        "         label=\"%s: %zu antenas e %zu rotas em coordenadas reais; "
        "%zu antena(s) fora do recorte. %zu cruzamento(s) visivel(eis), "
        "em vermelho.\"];\n"
        "  node [shape=circle, style=filled, fillcolor=\"#dbeafe\",\n"
        "        color=\"#2563eb\", penwidth=0.8, fixedsize=true,\n"
        "        width=%.3f, fontsize=%.0f, fontname=\"Helvetica\"];\n"
        "  edge [color=\"#94a3b8\", penwidth=%.2f];\n",
        regiao, vertices, fora, cruzamentos,
        regiao, vertices, arestas, fora, cruzamentos, largura, fonte,
        largura * 12.0 < 0.3 ? 0.3 : largura * 12.0);
}

static int escrever_vertices(FILE *arquivo, const Grafo *grafo,
    const Selecao *selecao, const Caixa *caixa, double escala, double largura)
{
    size_t total = grafo_quantidade_vertices(grafo);
    size_t i;

    for (i = 0U; i < total; ++i) {
        Vertice vertice;
        double x, y;
        if (!vertice_incluido(selecao, i)) continue;
        if (!grafo_obter_vertice(grafo, i, &vertice)) return 0;
        x = (vertice.coordenadas.longitude - caixa->oeste) * escala;
        y = (vertice.coordenadas.latitude - caixa->sul) * escala;
        fprintf(arquivo, "  n%zu [pos=\"%.5f,%.5f!\", label=\"%zu\"", i, x, y, i);
        if (selecao->vertices_cruzados[i]) {
            fprintf(arquivo, ", fillcolor=\"#dc2626\", color=\"#7f1d1d\", "
                "fontcolor=\"#ffffff\", penwidth=0.8, width=%.4f",
                largura * 2.0);
        }
        fprintf(arquivo, ", tooltip=\"antena %zu | celula %llu | mcc %u net %u "
            "area %u | lat %.4f lon %.4f | alcance %.0f m%s\"];\n",
            i, vertice.cell, vertice.mcc, vertice.net, vertice.area,
            vertice.coordenadas.latitude, vertice.coordenadas.longitude,
            vertice.alcance_metros,
            selecao->vertices_cruzados[i] ? " | EXTREMO DE CRUZAMENTO" : "");
    }
    return 1;
}

static int escrever_arestas(FILE *arquivo, const Grafo *grafo,
    const Selecao *selecao, double largura)
{
    size_t total = grafo_quantidade_arestas(grafo);
    size_t i;

    for (i = 0U; i < total; ++i) {
        size_t origem, destino;
        if (!grafo_obter_aresta(grafo, i, &origem, &destino)) return 0;
        if (!vertice_incluido(selecao, origem) ||
            !vertice_incluido(selecao, destino)) continue;
        fprintf(arquivo, "  n%zu -- n%zu", origem, destino);
        if (selecao->arestas_cruzadas != NULL && selecao->arestas_cruzadas[i])
            fprintf(arquivo, " [color=\"#dc2626\", penwidth=%.2f, "
                "tooltip=\"aresta %zu: envolvida em cruzamento\"]",
                largura * 30.0 < 0.6 ? 0.6 : largura * 30.0, i);
        else
            fprintf(arquivo, " [tooltip=\"aresta %zu\"]", i);
        fputs(";\n", arquivo);
    }
    return 1;
}

/* --- Gravacao ------------------------------------------------------------ */

static int gravar_mapa(const Grafo *grafo, const Cruzamento *cruzamentos,
    size_t quantidade_cruzamentos, const char *caminho, const Caixa *caixa,
    const char *regiao, int tamanho_por_quantidade)
{
    Selecao selecao;
    FILE *arquivo;
    size_t exibidos, rotas, fora, visiveis;
    double largura_desenho, altura_desenho, maior, escala, largura;

    memset(&selecao, 0, sizeof(selecao));
    if (grafo == NULL || caminho == NULL || caixa == NULL) return 0;
    if (grafo_quantidade_vertices(grafo) == 0U) return 0;
    if (!marcar_cruzamentos(grafo, cruzamentos, quantidade_cruzamentos,
            &selecao) ||
        !selecionar_na_caixa(grafo, &selecao, caixa, &fora)) {
        selecao_liberar(&selecao);
        return 0;
    }
    exibidos = contar_incluidos(grafo, &selecao);
    if (exibidos == 0U) {
        selecao_liberar(&selecao);
        return 0;
    }
    largura_desenho = caixa->leste - caixa->oeste;
    altura_desenho = caixa->norte - caixa->sul;
    maior = largura_desenho > altura_desenho ? largura_desenho : altura_desenho;
    escala = maior > 0.0 ? LADO_MAPA / maior : 1.0;
    largura = tamanho_por_quantidade ? largura_por_quantidade(exibidos)
        : largura_vertice(grafo, &selecao, escala, maior);

    arquivo = utf8_fopen(caminho, "w");
    if (arquivo == NULL) {
        selecao_liberar(&selecao);
        return 0;
    }
    rotas = contar_arestas_incluidas(grafo, &selecao);
    visiveis = contar_cruzamentos_visiveis(grafo, &selecao, cruzamentos,
        quantidade_cruzamentos);
    escrever_cabecalho_mapa(arquivo, regiao, exibidos, rotas, visiveis, fora,
        largura);
    if (!escrever_vertices(arquivo, grafo, &selecao, caixa, escala, largura) ||
        !escrever_arestas(arquivo, grafo, &selecao, largura)) {
        fclose(arquivo);
        selecao_liberar(&selecao);
        return 0;
    }
    fputs("}\n", arquivo);
    selecao_liberar(&selecao);
    if (ferror(arquivo)) {
        fclose(arquivo);
        return 0;
    }
    return fclose(arquivo) == 0;
}

int exportar_dot(const Grafo *grafo, const Cruzamento *cruzamentos,
    size_t quantidade_cruzamentos, const char *caminho)
{
    Caixa caixa;
    if (grafo == NULL || caminho == NULL) return 0;
    if (!caixa_total(grafo, &caixa)) return 0;
    return gravar_mapa(grafo, cruzamentos, quantidade_cruzamentos, caminho,
        &caixa, "Area completa", 1);
}

/* Monta "base_sufixo.ext" a partir de "base.ext". */
static int montar_caminho(char *destino, size_t tamanho, const char *base,
    const char *sufixo)
{
    const char *ponto = strrchr(base, '.');
    size_t prefixo = ponto != NULL ? (size_t)(ponto - base) : strlen(base);
    int escritos = snprintf(destino, tamanho, "%.*s_%s%s", (int)prefixo, base,
        sufixo, ponto != NULL ? ponto : ".dot");
    return escritos > 0 && (size_t)escritos < tamanho;
}

size_t exportar_dot_mapas(const Grafo *grafo, const Cruzamento *cruzamentos,
    size_t quantidade_cruzamentos, const char *caminho)
{
    static const char *sufixos[5] = {
        "1_regiao", "2_noroeste", "3_nordeste", "4_sudoeste", "5_sudeste"
    };
    static const char *nomes[5] = {
        "Regiao de maior concentracao", "Quadrante noroeste",
        "Quadrante nordeste", "Quadrante sudoeste", "Quadrante sudeste"
    };
    Caixa central, recortes[5];
    char destino[1024];
    double meio_longitude, meio_latitude;
    size_t i, gravados = 0U;

    if (grafo == NULL || caminho == NULL) return 0U;
    if (!caixa_central(grafo, &central)) return 0U;
    meio_longitude = (central.oeste + central.leste) / 2.0;
    meio_latitude = (central.sul + central.norte) / 2.0;
    recortes[0] = central;
    /* noroeste */
    recortes[1].oeste = central.oeste;  recortes[1].leste = meio_longitude;
    recortes[1].sul = meio_latitude;    recortes[1].norte = central.norte;
    /* nordeste */
    recortes[2].oeste = meio_longitude; recortes[2].leste = central.leste;
    recortes[2].sul = meio_latitude;    recortes[2].norte = central.norte;
    /* sudoeste */
    recortes[3].oeste = central.oeste;  recortes[3].leste = meio_longitude;
    recortes[3].sul = central.sul;      recortes[3].norte = meio_latitude;
    /* sudeste */
    recortes[4].oeste = meio_longitude; recortes[4].leste = central.leste;
    recortes[4].sul = central.sul;      recortes[4].norte = meio_latitude;

    for (i = 0U; i < 5U; ++i) {
        if (!montar_caminho(destino, sizeof(destino), caminho, sufixos[i]))
            continue;
        if (gravar_mapa(grafo, cruzamentos, quantidade_cruzamentos, destino,
            &recortes[i], nomes[i], 0)) ++gravados;
    }
    return gravados;
}