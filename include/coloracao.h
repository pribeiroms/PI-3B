#ifndef COLORACAO_H
#define COLORACAO_H

#include <stddef.h>
#include <stdio.h>

#include "grafo.h"

/* Resultado da coloracao: cada cor representa uma faixa de frequencia
 * (canal de radio). cores[v] guarda a cor do vertice v, numerada a partir de 0,
 * e total_cores indica quantos canais distintos foram necessarios.
 */
typedef struct {
    size_t *cores;
    size_t quantidade_vertices;
    size_t total_cores;
} ResultadoColoracao;

/* Ordem em que a coloracao gulosa visita os vertices. */
typedef enum {
    COLORACAO_ORDEM_NATURAL,   /* 0, 1, ..., V-1 */
    COLORACAO_WELSH_POWELL     /* grau decrescente; empate pelo menor indice */
} OrdemColoracao;

/* Numero de cores obtido em cada ordem, para o mesmo grafo. */
typedef struct {
    size_t cores_ordem_natural;
    size_t cores_welsh_powell;
} ComparacaoColoracao;

/* Coloracao gulosa de vertices: percorre os vertices na ordem natural
 * (0, 1, ..., V-1) e atribui a cada um a menor cor nao usada pelos vizinhos ja
 * coloridos. Funciona com Lista, Matriz ou estrutura conjunta.
 *
 * Retorna 1 em sucesso e 0 em erro. Em sucesso, o chamador deve liberar o
 * resultado com coloracao_liberar.
 */
int coloracao_gulosa(const Grafo *grafo, ResultadoColoracao *resultado);

/* Mesma coloracao gulosa, visitando os vertices na ordem escolhida. */
int coloracao_gulosa_ordem(const Grafo *grafo, OrdemColoracao ordem,
    ResultadoColoracao *resultado);

/* Preenche graus[v] com o grau de cada vertice. O vetor deve ter
 * grafo_quantidade_vertices(grafo) posicoes. Retorna 1 em sucesso.
 */
int coloracao_calcular_graus(const Grafo *grafo, size_t *graus);

/* Preenche ordem[] com os vertices em grau decrescente (Welsh-Powell). Vertices
 * de mesmo grau ficam pelo menor indice. Retorna 1 em sucesso.
 */
int coloracao_ordem_welsh_powell(const Grafo *grafo, size_t *ordem);

/* Executa as duas ordens e informa quantas cores cada uma usou. */
int coloracao_comparar_ordens(const Grafo *grafo, ComparacaoColoracao *comparacao);

/* Resultado da coloracao exata. Os limites interrompem a busca e sao
 * informados pelo status; nesse caso o resultado nao e preenchido.
 */
typedef enum {
    COLORACAO_EXATA_OK,
    COLORACAO_EXATA_LIMITE_TAMANHO,  /* grafo maior que max_vertices */
    COLORACAO_EXATA_LIMITE_TEMPO,    /* busca passou de max_segundos */
    COLORACAO_EXATA_ERRO
} StatusColoracaoExata;

/* Valores 0 desativam o respectivo limite. */
typedef struct {
    size_t max_vertices;
    double max_segundos;
} LimitesColoracaoExata;

/* Dados da execucao, preenchidos inclusive quando ha interrupcao.
 * limite_inferior: menor k ainda nao descartado. Em COLORACAO_EXATA_OK e o
 * numero cromatico; no limite de tempo, provou-se que k-1 cores nao bastam.
 */
typedef struct {
    double segundos;
    size_t limite_inferior;
    unsigned long long nos_visitados;
} InfoColoracaoExata;

/* Coloracao exata por backtracking: testa k = 1, 2, ... ate encontrar uma
 * coloracao valida, de modo que total_cores e o numero cromatico do grafo.
 * Os ramos em que um vizinho ja usa a cor sao podados. limites e info podem
 * ser NULL. Em COLORACAO_EXATA_OK o chamador libera com coloracao_liberar.
 */
StatusColoracaoExata coloracao_exata(const Grafo *grafo,
    const LimitesColoracaoExata *limites, ResultadoColoracao *resultado,
    InfoColoracaoExata *info);

const char *coloracao_mensagem_exata(StatusColoracaoExata status);

/* Resultado da verificacao de uma coloracao (gulosa ou exata). */
typedef struct {
    int valida;
    size_t arestas_em_conflito;   /* arestas cujos extremos tem a mesma cor */
    size_t primeira_aresta_conflito;  /* indice da primeira; so vale se houver */
    int cores_consistentes;       /* toda cor < total_cores e todas usadas */
} ValidacaoColoracao;

/* Verifica que nenhuma aresta liga dois vertices da mesma cor e que as cores
 * batem com total_cores. Retorna 1 se a verificacao pode ser feita (o resultado
 * corresponde ao grafo) e 0 em erro; a conclusao fica em validacao->valida.
 */
int coloracao_validar(const Grafo *grafo, const ResultadoColoracao *resultado,
    ValidacaoColoracao *validacao);

/* Retorna 1 se nenhum par de vertices adjacentes recebeu a mesma cor. */
int coloracao_valida(const Grafo *grafo, const ResultadoColoracao *resultado);

/* Faixa de espectro a partir da qual os canais sao alocados, em sequencia. */
typedef struct {
    double frequencia_inicial_mhz;
    double largura_canal_mhz;
} PlanoFrequencias;

/* Cor c vira o canal c + 1, ocupando
 * [inicial + c * largura, inicial + (c + 1) * largura) MHz.
 */
typedef struct {
    size_t numero;
    double inicio_mhz;
    double fim_mhz;
    size_t torres;
} CanalFrequencia;

/* Monta um canal por cor e conta as torres de cada um. *canais recebe um vetor
 * com total_cores posicoes, liberado pelo chamador com free. Retorna a
 * quantidade de canais (0 em grafo vazio, com *canais == NULL) e preenche
 * *erro com 1 se a entrada for invalida ou faltar memoria.
 */
size_t coloracao_mapear_canais(const ResultadoColoracao *resultado,
    PlanoFrequencias plano, CanalFrequencia **canais, int *erro);

/* Relatorio com a faixa e a quantidade de torres de cada canal. */
void coloracao_exibir_canais(FILE *saida, const CanalFrequencia *canais,
    size_t quantidade);

void coloracao_liberar(ResultadoColoracao *resultado);

#endif
