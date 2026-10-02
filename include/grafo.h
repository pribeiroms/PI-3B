#ifndef GRAFO_H
#define GRAFO_H

#include <stddef.h>

typedef struct Grafo Grafo;

typedef struct {
    double latitude;
    double longitude;
} CoordenadaGeografica;

typedef struct {
    size_t id;
    unsigned int mcc;
    unsigned int net;
    unsigned int area;
    unsigned int cell;
    CoordenadaGeografica coordenadas;
    double alcance_metros;
} Vertice;

typedef struct {
    size_t origem;
    size_t destino;
} Aresta;

typedef struct NoAdjacencia NoAdjacencia;

struct NoAdjacencia {
    size_t vertice;
    NoAdjacencia *proximo;
};

typedef struct {
    NoAdjacencia **listas;
    size_t quantidade_vertices;
} ListaAdjacencia;

typedef struct {
    unsigned char *dados;
    size_t ordem;
} MatrizAdjacencia;

typedef struct {
    CoordenadaGeografica inicio;
    CoordenadaGeografica fim;
} Segmento;

typedef struct {
    size_t aresta_a;
    size_t aresta_b;
} Cruzamento;

typedef struct {
    size_t memoria_comum_bytes;
    size_t memoria_lista_total_bytes;
    size_t memoria_matriz_total_bytes;
} EstimativaMemoriaGrafo;

typedef enum {
    GRAFO_ESTRUTURA_CONJUNTA,
    GRAFO_LISTA_ADJACENCIA,
    GRAFO_MATRIZ_ADJACENCIA
} EstruturaGrafo;

Grafo *grafo_criar(void);
Grafo *grafo_criar_com_estrutura(EstruturaGrafo estrutura);
void grafo_destruir(Grafo *grafo);

int grafo_adicionar_vertice(Grafo *grafo, Vertice vertice);
int grafo_adicionar_antena(Grafo *grafo, unsigned int mcc, unsigned int net,
    unsigned int area, unsigned int cell, double latitude,
    double longitude, double alcance_metros);
int grafo_adicionar_antena_ex(Grafo *grafo, unsigned int mcc, unsigned int net,
    unsigned int area, unsigned int cell, double latitude,
    double longitude, double alcance_metros);
int grafo_adicionar_aresta(Grafo *grafo, size_t origem, size_t destino);

size_t grafo_carregar_csv(Grafo *grafo, const char *caminho, size_t max_antenas,
    char *erro, size_t tamanho_erro);

size_t grafo_construir_conexoes(Grafo *grafo);
int grafo_construir_conexoes_ex(Grafo *grafo, size_t *arestas_construidas);

size_t grafo_quantidade_vertices(const Grafo *grafo);
size_t grafo_quantidade_arestas(const Grafo *grafo);

int grafo_possui_cruzamentos(const Grafo *grafo);
int grafo_sao_adjacentes(const Grafo *grafo, size_t origem, size_t destino);
const NoAdjacencia *grafo_vizinhos(const Grafo *grafo, size_t vertice);

typedef enum{
    EULER_NAO_APLICAVEL,
    EULER_NAO_PLANAR,
    EULER_INCONCLUSIVO
}ResultadoEuler;

ResultadoEuler grafo_verificar_euler( const Grafo *grafo);
const char *grafo_mensagem_euler(ResultadoEuler resultado);

size_t grafo_detectar_cruzamentos(const Grafo *grafo, Cruzamento **cruzamentos);

int grafo_estimar_memoria(const Grafo *grafo, EstimativaMemoriaGrafo *estimativa);

/* --- Restauradas: removidas por engano na PR #58 (vieram das #14/#15/#16) --- */

/* Obtem o segmento (coordenadas) de uma aresta. */
int grafo_obter_segmento(const Grafo *grafo, size_t indice_aresta, Segmento *saida);

/* Retorna 1 se as duas arestas compartilham algum vertice. */
int grafo_arestas_compartilham_vertice(const Grafo *grafo, size_t aresta_a, size_t aresta_b);

/* Retorna 1 se os dois segmentos se cruzam (caso geral ou colinear). */
int grafo_segmentos_se_cruzam(Segmento a, Segmento b);

/* Obtem os indices das antenas (origem/destino) de uma aresta. */
int grafo_obter_aresta(const Grafo *grafo, size_t indice_aresta, size_t *origem, size_t *destino);

#endif