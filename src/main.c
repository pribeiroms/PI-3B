#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dataset.h"
#include "grafo.h"
#include "analise_planaridade.h"

static double decorrido_ms(clock_t inicio, clock_t fim)
{
    return (double)(fim - inicio) * 1000.0 / CLOCKS_PER_SEC;
}

int main(int argc, char *argv[])
{
    const char *caminho = argc > 1 ? argv[1] : "data/opencellid_brasil_filtrado.csv";
    size_t limite = argc > 2 ? (size_t)strtoul(argv[2], NULL, 10) : 1000U;
    char erro[128];
    Grafo *grafo = grafo_criar();
    ResultadoEuler euler;
    RelatorioCarregamento relatorio;
    clock_t inicio, fim;
    int cruzamentos;

    if (grafo == NULL) {
        fputs("Erro ao criar o grafo.\n", stderr);
        return 1;
    }

    inicio = clock();
    if (!dataset_carregar_opencellid(grafo, caminho, limite, &relatorio, erro, sizeof(erro))) {
        fprintf(stderr, "Erro ao carregar dataset: %s\n", erro);
        grafo_destruir(grafo);
        return 1;
    }
    fim = clock();
    printf("Leitura do dataset: %.3f ms (Lista de Adjacencia)\n", decorrido_ms(inicio, fim));

    if (relatorio.registros_carregados == 0U) {
        fputs("Erro ao carregar dataset: nenhum registro valido encontrado.\n", stderr);
        grafo_destruir(grafo);
        return 1;
    }
    printf("Registros invalidos ignorados: %zu\n", relatorio.registros_invalidos);

    inicio = clock();
    (void)grafo_construir_conexoes(grafo);
    fim = clock();
    printf("Construcao do grafo: %.3f ms (Lista de Adjacencia)\n", decorrido_ms(inicio, fim));

    inicio = clock();
    euler = grafo_verificar_euler(grafo);
    fim = clock();
    printf("Validacao por Euler: %.3f ms (Lista de Adjacencia)\n", decorrido_ms(inicio, fim));

    inicio = clock();
    cruzamentos = grafo_possui_cruzamentos(grafo);
    fim = clock();
    printf("Analise de cruzamentos: %.3f ms (Lista de Adjacencia)\n", decorrido_ms(inicio, fim));

    /* #16 ainda retorna apenas presenca; nao converter esse booleano em contagem. */
    analise_planaridade_exibir(stdout, grafo_quantidade_vertices(grafo),
        grafo_quantidade_arestas(grafo), euler, cruzamentos, NULL);

    grafo_destruir(grafo);
    return 0;
}
