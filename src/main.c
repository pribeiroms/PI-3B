#include <stdio.h>
#include <stdlib.h>

#include "dataset.h"
#include "grafo.h"

int main(int argc, char *argv[])
{
    const char *caminho = argc > 1 ? argv[1] : "data/opencellid_brasil_filtrado.csv";
    size_t limite = argc > 2 ? (size_t)strtoul(argv[2], NULL, 10) : 1000U;
    char erro[128];
    Grafo *grafo = grafo_criar();
    RelatorioCarregamento relatorio;

    if (grafo == NULL) {
        fputs("Erro ao criar o grafo.\n", stderr);
        return 1;
    }
    if (!dataset_carregar_opencellid(grafo, caminho, limite, &relatorio, erro, sizeof(erro))) {
        fprintf(stderr, "Erro ao carregar dataset: %s\n", erro);
        grafo_destruir(grafo);
        return 1;
    }
    if (relatorio.registros_carregados == 0U) {
        fputs("Erro ao carregar dataset: nenhum registro valido encontrado.\n", stderr);
        grafo_destruir(grafo);
        return 1;
    }
    printf("Registros invalidos ignorados: %zu\n", relatorio.registros_invalidos);
    (void)grafo_construir_conexoes(grafo);
    printf("Vertices: %zu\nArestas sem peso: %zu\n", grafo_quantidade_vertices(grafo),
           grafo_quantidade_arestas(grafo));
    if (grafo_possui_cruzamentos(grafo)) {
        puts("Resultado: existem cruzamentos; a planta exige isolamento ou novas rotas.");
    } else {
        puts("Resultado: nao foram encontrados cruzamentos na planta analisada.");
    }
    grafo_destruir(grafo);
    return 0;
}
