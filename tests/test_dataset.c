#include <assert.h>
#include <stdio.h>

#include "dataset.h"

static void teste_carrega_csv_e_registra_invalidos(void)
{
    Grafo *grafo = grafo_criar();
    RelatorioCarregamento relatorio;
    char erro[128];

    assert(grafo != NULL);
    assert(dataset_carregar_opencellid(grafo, "tests/dados_dataset.csv", 0U,
           &relatorio, erro, sizeof(erro)) == 1);
    assert(relatorio.linhas_lidas == 4U);
    assert(relatorio.registros_carregados == 2U);
    assert(relatorio.registros_invalidos == 2U);
    assert(grafo_quantidade_vertices(grafo) == 2U);
    grafo_destruir(grafo);
}

static void teste_erro_ao_abrir_arquivo(void)
{
    Grafo *grafo = grafo_criar();
    RelatorioCarregamento relatorio;
    char erro[128];

    assert(grafo != NULL);
    assert(dataset_carregar_opencellid(grafo, "tests/inexistente.csv", 0U,
           &relatorio, erro, sizeof(erro)) == 0);
    assert(erro[0] != '\0');
    grafo_destruir(grafo);
}

static void teste_carrega_mil_registros_do_dataset_real(void)
{
    Grafo *grafo = grafo_criar();
    RelatorioCarregamento relatorio;
    char erro[128];

    assert(grafo != NULL);
    assert(dataset_carregar_opencellid(grafo, "data/opencellid_brasil_filtrado.csv", 1000U,
           &relatorio, erro, sizeof(erro)) == 1);
    assert(relatorio.registros_carregados == 1000U);
    assert(grafo_quantidade_vertices(grafo) == 1000U);
    grafo_destruir(grafo);
}

int main(void)
{
    teste_carrega_csv_e_registra_invalidos();
    teste_erro_ao_abrir_arquivo();
    teste_carrega_mil_registros_do_dataset_real();
    puts("Testes de dataset passaram.");
    return 0;
}
