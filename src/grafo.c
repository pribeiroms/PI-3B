#include <stdlib.h>

#include "grafo.h"

/*
 * Placeholder mínimo: substitua pela representação autoral escolhida
 * (por exemplo, lista de adjacência ou matriz de adjacência).
 */
struct Grafo {
    int reservado;
};

Grafo *grafo_criar(void)
{
    return calloc(1, sizeof(Grafo));
}

void grafo_destruir(Grafo *grafo)
{
    free(grafo);
}
