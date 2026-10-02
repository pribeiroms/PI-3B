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

/* Cria um grafo vazio. Retorna NULL caso a alocação falhe. */
Grafo *grafo_criar(void);

/* Libera todos os recursos associados ao grafo. */
void grafo_destruir(Grafo *grafo);

/* Adiciona um vertice e retorna seu indice, ou -1 caso a entrada seja invalida. */
int grafo_adicionar_vertice(Grafo *grafo, Vertice vertice);

/* Adiciona uma antena georreferenciada e retorna seu indice, ou -1 em erro. */
int grafo_adicionar_antena(Grafo *grafo, unsigned int mcc, unsigned int net,
    unsigned int area, unsigned int cell, double latitude,
    double longitude, double alcance_metros);

/* Adiciona uma aresta nao direcionada e sem peso. Retorna 1 em caso de sucesso. */
int grafo_adicionar_aresta(Grafo *grafo, size_t origem, size_t destino);

/* Le antenas de um CSV OpenCelliD; max_antenas igual a 0 le todos os registros. */
size_t grafo_carregar_csv(Grafo *grafo, const char *caminho, size_t max_antenas,
    char *erro, size_t tamanho_erro);

/* Cria as arestas da Fase I conforme os criterios de proximidade e alcance. */
size_t grafo_construir_conexoes(Grafo *grafo);

size_t grafo_quantidade_vertices(const Grafo *grafo);
size_t grafo_quantidade_arestas(const Grafo *grafo);

/* Retorna 1 se existir cruzamento entre duas arestas sem vertice em comum. */
int grafo_possui_cruzamentos(const Grafo *grafo);

int grafo_sao_adjacentes(const Grafo *grafo, size_t origem, size_t destino);

/* Retorna o inicio da lista ligada de vizinhos de um vertice, ou NULL se invalido/sem vizinhos. */
const NoAdjacencia *grafo_vizinhos(const Grafo *grafo, size_t vertice);

/* Criar os Resultados das validacoes de planaridade com base na fórmula de Euler*/
typedef enum{
    EULER_NAO_APLICAVEL, /*Nao se aplica a formula */
    EULER_NAO_PLANAR, /*E > 3V-6: certamente não é planar*/
    EULER_INCONCLUSIVO /*Não tem certeza de Planaridade*/
}ResultadoEuler;

/*Função que verifica a condição da formula E <= 3V - 6 (Grafo Simples)*/
ResultadoEuler grafo_verificar_euler( const Grafo *grafo);

/*Retorna um texto explicativo do resultado, para exibir ao usuario*/
const char *grafo_mensagem_euler(ResultadoEuler resultado);

/*Obtem o segmento de duas antenas de uma aresta*/
int grafo_obter_segmento(const Grafo *grafo, size_t indice_aresta, Segmento *saida);

int grafo_arestas_compartilham_vertice(const Grafo *grafo, size_t aresta_a, size_t aresta_b);

/* Retorna 1 se os dois segmentos se cruzam (caso geral ou colinear), 0 caso contrario. */
int grafo_segmentos_se_cruzam(Segmento a, Segmento b);

#endif