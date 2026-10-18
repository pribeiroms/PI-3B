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
    double peso;
} Aresta;

typedef struct NoAdjacencia NoAdjacencia;

struct NoAdjacencia {
    size_t vertice;
    double peso;
    NoAdjacencia *proximo;
};

typedef struct {
    NoAdjacencia **listas;
    size_t quantidade_vertices;
} ListaAdjacencia;

typedef struct {
    unsigned char *dados;  /* adjacencia, um bit por par de vertices */
    double *pesos;         /* pesos; NULL ate a primeira aresta com peso */
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

/* Copia os dados de um vertice (antena) pelo indice. Retorna 1 em sucesso. */
int grafo_obter_vertice(const Grafo *grafo, size_t indice, Vertice *saida);

/* --- Fase II --- */

/* Distancia geodesica entre duas coordenadas, em quilometros.
 *
 * Formula de Haversine com raio terrestre de 6.371 km. Esta e a unica
 * implementacao de distancia do projeto: a construcao das conexoes e a regra
 * de interferencia derivam dela. A unidade e a justificativa estao em
 * docs/MODELAGEM_PESOS_FASE2.md (issue #65).
 *
 * A funcao e simetrica e devolve 0 para coordenadas iguais. Coordenadas fora
 * do dominio valido nao sao rejeitadas aqui; a validacao ocorre na carga do
 * dataset.
 */
double grafo_distancia_km(CoordenadaGeografica a, CoordenadaGeografica b);

/* Insere uma aresta com peso, em quilometros.
 *
 * O peso e gravado nas tres representacoes mantidas pelo grafo: no vetor de
 * arestas, nos nos da lista de adjacencia e, quando a estrutura inclui a
 * matriz, numa matriz de pesos paralela a de adjacencia.
 *
 * A matriz de pesos e alocada apenas na primeira aresta com peso diferente de
 * zero: um grafo sem peso, como o da Fase I, nao paga esse custo. Ela ocupa
 * ordem x ordem doubles, o que a torna o fator dominante de memoria em grafos
 * valorados grandes.
 *
 * Pesos negativos, infinitos ou NaN sao rejeitados. Laços e arestas repetidas
 * continuam rejeitados, como em grafo_adicionar_aresta.
 *
 * Retorna 1 em sucesso e 0 em erro.
 */
int grafo_adicionar_aresta_ponderada(Grafo *grafo, size_t origem,
    size_t destino, double peso);

/* Consulta o peso da aresta entre dois vertices, em quilometros.
 *
 * Usa a representacao ativa: a matriz responde em O(1), a lista em O(grau).
 * Retorna 0 se a aresta nao existir, e nesse caso nao escreve em peso.
 */
int grafo_obter_peso(const Grafo *grafo, size_t origem, size_t destino,
    double *peso);

/* Obtem origem, destino e peso de uma aresta pelo indice no vetor de arestas.
 * Qualquer um dos ponteiros de saida pode ser NULL. Retorna 1 em sucesso.
 *
 * E a forma indicada de percorrer as arestas para ordena-las, como faz o
 * Kruskal.
 */
int grafo_obter_aresta_ponderada(const Grafo *grafo, size_t indice_aresta,
    size_t *origem, size_t *destino, double *peso);

/* Soma dos pesos de todas as arestas, em quilometros. Em uma arvore geradora,
 * e a metragem total de cabo do traçado. */
double grafo_peso_total(const Grafo *grafo);

#endif