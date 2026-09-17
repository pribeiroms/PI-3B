#include <stdio.h>

#include "grafo.h"

int main(void)
{
    Grafo *grafo = grafo_criar();

    if (grafo == NULL) {
        fputs("Erro ao criar o grafo.\n", stderr);
        return 1;
    }

    puts("Projeto de grafos inicializado.");
    grafo_destruir(grafo);
    return 0;
}
