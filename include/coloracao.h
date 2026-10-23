#ifndef COLORACAO_H
#define COLORACAO_H

#include <stddef.h>

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

/* Coloracao gulosa de vertices: percorre os vertices na ordem natural
 * (0, 1, ..., V-1) e atribui a cada um a menor cor nao usada pelos vizinhos ja
 * coloridos. Funciona com Lista, Matriz ou estrutura conjunta.
 *
 * Retorna 1 em sucesso e 0 em erro. Em sucesso, o chamador deve liberar o
 * resultado com coloracao_liberar.
 */
int coloracao_gulosa(const Grafo *grafo, ResultadoColoracao *resultado);

/* Retorna 1 se nenhum par de vertices adjacentes recebeu a mesma cor. */
int coloracao_valida(const Grafo *grafo, const ResultadoColoracao *resultado);

void coloracao_liberar(ResultadoColoracao *resultado);

#endif
