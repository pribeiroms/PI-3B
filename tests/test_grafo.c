#include <assert.h>
#include <stdio.h>

#include "grafo.h"

static void teste_cria_e_destroi_grafo(void)
{
    Grafo *grafo = grafo_criar();

    assert(grafo != NULL);
    grafo_destruir(grafo);
}

int main(void)
{
    teste_cria_e_destroi_grafo();
    puts("Todos os testes passaram.");
    return 0;
}
