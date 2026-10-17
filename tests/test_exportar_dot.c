#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exportar_dot.h"

#define ARQUIVO "build/test_exportar_dot.tmp"

/* Le o arquivo exportado inteiro para memoria. O chamador libera o retorno. */
static char *ler_arquivo(const char *caminho)
{
    FILE *arquivo = fopen(caminho, "rb");
    char *texto;
    size_t lidos;

    assert(arquivo != NULL);
    texto = malloc(65536U);
    assert(texto != NULL);
    lidos = fread(texto, 1U, 65535U, arquivo);
    assert(!ferror(arquivo));
    texto[lidos] = '\0';
    fclose(arquivo);
    return texto;
}

/* Quatro antenas na mesma area, proximas o bastante para se conectarem. */
static Grafo *criar_grafo_de_teste(void)
{
    Grafo *grafo = grafo_criar();
    assert(grafo != NULL);
    /* grafo_adicionar_antena devolve o indice do vertice, logo 0 e sucesso. */
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 1U, 10U, 0.0, 0.0, 5000.0) == 0);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 1U, 11U, 0.0, 0.01, 5000.0) == 1);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 1U, 12U, 0.01, 0.0, 5000.0) == 2);
    assert(grafo_adicionar_antena(grafo, 724U, 2U, 1U, 13U, 0.01, 0.01, 5000.0) == 3);
    return grafo;
}

static void teste_exporta_vertices_e_arestas(void)
{
    Grafo *grafo = criar_grafo_de_teste();
    char *texto;

    assert(grafo_adicionar_aresta(grafo, 0U, 1U));
    assert(grafo_adicionar_aresta(grafo, 2U, 3U));
    assert(exportar_dot(grafo, NULL, 0U, ARQUIVO));
    texto = ler_arquivo(ARQUIVO);
    assert(strstr(texto, "graph Mapa {") != NULL);
    assert(strstr(texto, "layout=neato") != NULL);
    assert(strstr(texto, "Area completa") != NULL);
    /* Posicoes geograficas fixas. */
    assert(strstr(texto, "n0 [pos=") != NULL);
    assert(strstr(texto, "n3 [pos=") != NULL);
    assert(strstr(texto, "n0 -- n1") != NULL);
    assert(strstr(texto, "n2 -- n3") != NULL);
    /* Sem cruzamentos informados, nada e destacado. */
    assert(strstr(texto, "#dc2626") == NULL);
    free(texto);
    grafo_destruir(grafo);
    assert(remove(ARQUIVO) == 0);
}

static void teste_destaca_cruzamentos(void)
{
    Grafo *grafo = criar_grafo_de_teste();
    Cruzamento cruzamento;
    char *texto;

    /* As diagonais do quadrado se cruzam. */
    assert(grafo_adicionar_aresta(grafo, 0U, 3U));
    assert(grafo_adicionar_aresta(grafo, 1U, 2U));
    cruzamento.aresta_a = 0U;
    cruzamento.aresta_b = 1U;
    assert(exportar_dot(grafo, &cruzamento, 1U, ARQUIVO));
    texto = ler_arquivo(ARQUIVO);
    assert(strstr(texto, "1 cruzamento(s)") != NULL);
    /* As duas arestas e os quatro extremos saem destacados. */
    assert(strstr(texto, "envolvida em cruzamento") != NULL);
    assert(strstr(texto, "EXTREMO DE CRUZAMENTO") != NULL);
    assert(strstr(texto, "#dc2626") != NULL);
    free(texto);
    grafo_destruir(grafo);
    assert(remove(ARQUIVO) == 0);
}

static void teste_serie_de_recortes(void)
{
    Grafo *grafo = criar_grafo_de_teste();
    char *texto;

    assert(grafo_adicionar_aresta(grafo, 0U, 1U));
    assert(grafo_adicionar_aresta(grafo, 2U, 3U));
    /* Regiao central mais os quatro quadrantes. */
    assert(exportar_dot_mapas(grafo, NULL, 0U, "build/serie.dot") == 5U);
    texto = ler_arquivo("build/serie_1_regiao.dot");
    assert(strstr(texto, "Regiao de maior concentracao") != NULL);
    assert(strstr(texto, "pos=") != NULL);
    free(texto);
    texto = ler_arquivo("build/serie_3_nordeste.dot");
    assert(strstr(texto, "Quadrante nordeste") != NULL);
    free(texto);
    assert(remove("build/serie_1_regiao.dot") == 0);
    assert(remove("build/serie_2_noroeste.dot") == 0);
    assert(remove("build/serie_3_nordeste.dot") == 0);
    assert(remove("build/serie_4_sudoeste.dot") == 0);
    assert(remove("build/serie_5_sudeste.dot") == 0);
    grafo_destruir(grafo);
}

static void teste_entradas_invalidas(void)
{
    Grafo *vazio = grafo_criar();
    Grafo *grafo = criar_grafo_de_teste();

    assert(vazio != NULL);
    assert(!exportar_dot(NULL, NULL, 0U, ARQUIVO));
    assert(!exportar_dot(grafo, NULL, 0U, NULL));
    /* Grafo sem vertices nao gera desenho. */
    assert(!exportar_dot(vazio, NULL, 0U, ARQUIVO));
    /* Diretorio inexistente deve falhar sem travar. */
    assert(!exportar_dot(grafo, NULL, 0U, "build/nao_existe/x.dot"));
    assert(exportar_dot_mapas(NULL, NULL, 0U, ARQUIVO) == 0U);
    assert(exportar_dot_mapas(vazio, NULL, 0U, ARQUIVO) == 0U);
    assert(exportar_dot_mapas(grafo, NULL, 0U, NULL) == 0U);
    grafo_destruir(vazio);
    grafo_destruir(grafo);
}

static void teste_grafo_obter_vertice(void)
{
    Grafo *grafo = criar_grafo_de_teste();
    Vertice vertice;

    assert(grafo_obter_vertice(grafo, 0U, &vertice));
    assert(vertice.cell == 10U);
    assert(vertice.mcc == 724U);
    assert(grafo_obter_vertice(grafo, 3U, &vertice));
    assert(vertice.cell == 13U);
    /* Indice fora do intervalo e ponteiros nulos. */
    assert(!grafo_obter_vertice(grafo, 4U, &vertice));
    assert(!grafo_obter_vertice(grafo, 0U, NULL));
    assert(!grafo_obter_vertice(NULL, 0U, &vertice));
    grafo_destruir(grafo);
}

int main(void)
{
    teste_grafo_obter_vertice();
    teste_exporta_vertices_e_arestas();
    teste_destaca_cruzamentos();
    teste_serie_de_recortes();
    teste_entradas_invalidas();
    puts("Testes de exportacao DOT passaram.");
    return 0;
}