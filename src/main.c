#include <stdio.h>
#include <time.h>

#include "analise_planaridade.h"
#include "execucao.h"

static double decorrido_ms(clock_t inicio, clock_t fim)
{
    if (inicio == (clock_t)-1 || fim == (clock_t)-1) return -1.0;
    return (double)(fim - inicio) * 1000.0 / CLOCKS_PER_SEC;
}

static const char *nome_estrutura(EstruturaGrafo estrutura)
{
    switch (estrutura) {
        case GRAFO_LISTA_ADJACENCIA: return "lista de adjacencia";
        case GRAFO_MATRIZ_ADJACENCIA: return "matriz de adjacencia";
        default: return "lista e matriz simultaneas (conjunta)";
    }
}

int main(int argc, char *argv[])
{
    OpcoesExecucao opcoes;
    ResultadoExecucao resultado = {0};
    char erro[128];
    Grafo *grafo;
    size_t quantidade_cruzamentos;
    clock_t inicio, fim, inicio_total, fim_total;
    int status = execucao_ler_opcoes(argc, argv, &opcoes);
    if (status == 0) {
        execucao_exibir_ajuda();
        return 0;
    }
    if (status < 0) return 2;
    inicio_total = clock();
    grafo = grafo_criar_com_estrutura(opcoes.estrutura);
    if (grafo == NULL) {
        fputs("Erro ao criar o grafo.\n", stderr);
        return 1;
    }
    printf("Dataset: %s\nLimite de vertices: %zu (0 = todos)\n"
           "Estrutura: %s.\n", opcoes.dataset, opcoes.limite,
           nome_estrutura(opcoes.estrutura));
    inicio = clock();
    if (!dataset_carregar_opencellid(grafo, opcoes.dataset, opcoes.limite,
            &resultado.carregamento, erro, sizeof(erro))) {
        fprintf(stderr, "Erro ao carregar dataset: %s\n", erro);
        grafo_destruir(grafo);
        return 1;
    }
    fim = clock();
    resultado.leitura_ms = decorrido_ms(inicio, fim);
    if (resultado.carregamento.registros_carregados == 0U) {
        fputs("Erro ao carregar dataset: nenhum registro valido encontrado.\n", stderr);
        grafo_destruir(grafo);
        return 1;
    }
    inicio = clock();
    if (!grafo_construir_conexoes_ex(grafo, &resultado.arestas)) {
        fputs("Erro ao construir conexoes do grafo (memoria insuficiente).\n", stderr);
        grafo_destruir(grafo);
        return 1;
    }
    fim = clock();
    resultado.construcao_ms = decorrido_ms(inicio, fim);
    resultado.vertices = grafo_quantidade_vertices(grafo);

    inicio = clock();
    resultado.euler = grafo_verificar_euler(grafo);
    fim = clock();
    resultado.euler_ms = decorrido_ms(inicio, fim);
    inicio = clock();
    quantidade_cruzamentos = grafo_detectar_cruzamentos(grafo, NULL);
    resultado.quantidade_cruzamentos = quantidade_cruzamentos;
    resultado.possui_cruzamentos = quantidade_cruzamentos > 0U;
    fim = clock();
    resultado.cruzamentos_ms = decorrido_ms(inicio, fim);
    resultado.memoria_disponivel = grafo_estimar_memoria(grafo, &resultado.memoria);
    grafo_destruir(grafo);
    fim_total = clock();
    resultado.total_cpu_ms = decorrido_ms(inicio_total, fim_total);

    printf("Registros invalidos ignorados: %zu\n"
           "Tempos de CPU em ms (-1 = indisponivel):\n"
           "Leitura: %.3f\nConstrucao: %.3f\nEuler: %.3f\nCruzamentos: %.3f\n"
           "Total da execucao: %.3f\n",
           resultado.carregamento.registros_invalidos, resultado.leitura_ms,
           resultado.construcao_ms, resultado.euler_ms, resultado.cruzamentos_ms,
           resultado.total_cpu_ms);
    if (resultado.memoria_disponivel) {
        printf("Memoria estimada em bytes (modelo das alocacoes do grafo):\n"
               "Comum: %zu\nLista de adjacencia: %zu\nMatriz de adjacencia: %zu\n",
               resultado.memoria.memoria_comum_bytes,
               resultado.memoria.memoria_lista_total_bytes,
               resultado.memoria.memoria_matriz_total_bytes);
    } else {
        puts("Estimativa de memoria: indisponivel (overflow ou entrada invalida).");
    }
    analise_planaridade_exibir(stdout, resultado.vertices, resultado.arestas,
        resultado.euler, resultado.possui_cruzamentos,
        &resultado.quantidade_cruzamentos);
    if (!execucao_salvar(&opcoes, &resultado)) {
        fprintf(stderr, "Erro ao salvar resultados em %s. Verifique o diretorio, "
            "as permissoes e se o arquivo possui o cabecalho esperado.\n", opcoes.saida);
        return 1;
    }
    printf("Resultados acrescentados em: %s\n", opcoes.saida);
    return 0;
}
