/* Benchmark da coloracao de frequencias (issue #84).
 *
 * Executa uma unica medicao: carrega um subconjunto, constroi o grafo de
 * interferencia (#69) na estrutura pedida e roda um dos algoritmos de
 * coloracao, cronometrando apenas o algoritmo com a instrumentacao da #86.
 * As repeticoes e a varredura dos tamanhos ficam no roteiro
 * scripts/benchmark_coloracao.ps1, que le as linhas "chave: valor" impressas
 * aqui.
 *
 * Uso:
 *   benchmark_coloracao.exe --dataset ARQ --estrutura lista|matriz
 *       --algoritmo gulosa|welsh-powell|exata [--limite-segundos S]
 *       [--saida ARQ.csv]
 *
 * --limite-segundos vale so para a exata (padrao 60; 0 desativa).
 * --saida acrescenta o registro no CSV da Fase II (medicao_gravar_csv).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coloracao.h"
#include "dataset.h"
#include "grafo.h"
#include "interferencia.h"
#include "medicao.h"

typedef enum {
    ALGORITMO_GULOSA,
    ALGORITMO_WELSH_POWELL,
    ALGORITMO_EXATA
} AlgoritmoBenchmark;

typedef struct {
    const char *dataset;
    EstruturaGrafo estrutura;
    AlgoritmoBenchmark algoritmo;
    double limite_segundos;
    const char *saida;
} OpcoesBenchmark;

static void exibir_uso(void)
{
    fputs("Uso: benchmark_coloracao --dataset ARQ --estrutura lista|matriz "
        "--algoritmo gulosa|welsh-powell|exata [--limite-segundos S] "
        "[--saida ARQ.csv]\n", stderr);
}

static int ler_opcoes(int argc, char *argv[], OpcoesBenchmark *opcoes)
{
    int i, tem_estrutura = 0, tem_algoritmo = 0;

    opcoes->dataset = NULL;
    opcoes->estrutura = GRAFO_LISTA_ADJACENCIA;
    opcoes->algoritmo = ALGORITMO_GULOSA;
    opcoes->limite_segundos = 60.0;
    opcoes->saida = NULL;
    for (i = 1; i < argc; ++i) {
        const char *valor = i + 1 < argc ? argv[i + 1] : NULL;
        if (valor == NULL) return 0;
        if (strcmp(argv[i], "--dataset") == 0) {
            opcoes->dataset = valor;
        } else if (strcmp(argv[i], "--estrutura") == 0) {
            if (strcmp(valor, "lista") == 0) opcoes->estrutura = GRAFO_LISTA_ADJACENCIA;
            else if (strcmp(valor, "matriz") == 0) opcoes->estrutura = GRAFO_MATRIZ_ADJACENCIA;
            else return 0;
            tem_estrutura = 1;
        } else if (strcmp(argv[i], "--algoritmo") == 0) {
            if (strcmp(valor, "gulosa") == 0) opcoes->algoritmo = ALGORITMO_GULOSA;
            else if (strcmp(valor, "welsh-powell") == 0) opcoes->algoritmo = ALGORITMO_WELSH_POWELL;
            else if (strcmp(valor, "exata") == 0) opcoes->algoritmo = ALGORITMO_EXATA;
            else return 0;
            tem_algoritmo = 1;
        } else if (strcmp(argv[i], "--limite-segundos") == 0) {
            char *fim;
            opcoes->limite_segundos = strtod(valor, &fim);
            if (*fim != '\0' || opcoes->limite_segundos < 0.0) return 0;
        } else if (strcmp(argv[i], "--saida") == 0) {
            opcoes->saida = valor;
        } else {
            return 0;
        }
        ++i;
    }
    return opcoes->dataset != NULL && tem_estrutura && tem_algoritmo;
}

static const char *nome_algoritmo(AlgoritmoBenchmark algoritmo)
{
    switch (algoritmo) {
    case ALGORITMO_GULOSA: return "coloracao_gulosa";
    case ALGORITMO_WELSH_POWELL: return "coloracao_welsh_powell";
    default: return "coloracao_exata";
    }
}

/* Buffers de heap que o algoritmo aloca, pelo codigo de src/coloracao.c:
 * a gulosa usa cores, marca e sequencia (3 vetores de V posicoes); Welsh-Powell
 * soma graus e o vetor de pares (vertice, grau) durante a ordenacao; a exata
 * usa so o vetor de cores. A pilha da recursao da exata nao entra.
 */
static size_t memoria_auxiliar(AlgoritmoBenchmark algoritmo, size_t vertices)
{
    switch (algoritmo) {
    case ALGORITMO_GULOSA: return 3U * vertices * sizeof(size_t);
    case ALGORITMO_WELSH_POWELL: return 6U * vertices * sizeof(size_t);
    default: return vertices * sizeof(size_t);
    }
}

static StatusMedicao status_exata(StatusColoracaoExata status)
{
    switch (status) {
    case COLORACAO_EXATA_OK: return MEDICAO_STATUS_OK;
    case COLORACAO_EXATA_LIMITE_TEMPO: return MEDICAO_STATUS_LIMITE_TEMPO;
    case COLORACAO_EXATA_LIMITE_TAMANHO: return MEDICAO_STATUS_LIMITE_TAMANHO;
    default: return MEDICAO_STATUS_ERRO;
    }
}

int main(int argc, char *argv[])
{
    OpcoesBenchmark opcoes;
    RelatorioCarregamento carregamento;
    RelatorioInterferencia interferencia;
    ResultadoColoracao resultado = {NULL, 0U, 0U};
    InfoColoracaoExata info = {0.0, 0U, 0U};
    RegistroMedicao registro;
    Cronometro cronometro;
    char erro[128];
    Grafo *grafo;
    int valida = 0, tem_resultado = 0;

    if (!ler_opcoes(argc, argv, &opcoes)) {
        exibir_uso();
        return 2;
    }
    grafo = grafo_criar_com_estrutura(opcoes.estrutura);
    if (grafo == NULL) {
        fputs("Erro ao criar o grafo.\n", stderr);
        return 1;
    }
    if (!dataset_carregar_opencellid(grafo, opcoes.dataset, 0U, &carregamento,
            erro, sizeof(erro))) {
        fprintf(stderr, "Erro ao carregar dataset: %s\n", erro);
        grafo_destruir(grafo);
        return 1;
    }
    if (!interferencia_construir(grafo, &interferencia)) {
        fputs("Erro ao construir o grafo de interferencia.\n", stderr);
        grafo_destruir(grafo);
        return 1;
    }
    if (!medicao_preencher_do_grafo(&registro, nome_algoritmo(opcoes.algoritmo),
            grafo, 0U)) {
        fputs("Erro ao preencher a medicao.\n", stderr);
        grafo_destruir(grafo);
        return 1;
    }

    cronometro_iniciar(&cronometro);
    if (opcoes.algoritmo == ALGORITMO_EXATA) {
        LimitesColoracaoExata limites;
        StatusColoracaoExata status;
        limites.max_vertices = 0U;
        limites.max_segundos = opcoes.limite_segundos;
        status = coloracao_exata(grafo, &limites, &resultado, &info);
        registro.tempo_ms = cronometro_decorrido_ms(&cronometro);
        registro.status = status_exata(status);
        tem_resultado = status == COLORACAO_EXATA_OK;
    } else {
        OrdemColoracao ordem = opcoes.algoritmo == ALGORITMO_WELSH_POWELL
            ? COLORACAO_WELSH_POWELL : COLORACAO_ORDEM_NATURAL;
        tem_resultado = coloracao_gulosa_ordem(grafo, ordem, &resultado);
        registro.tempo_ms = cronometro_decorrido_ms(&cronometro);
        registro.status = tem_resultado ? MEDICAO_STATUS_OK : MEDICAO_STATUS_ERRO;
    }
    registro.memoria_auxiliar_bytes =
        memoria_auxiliar(opcoes.algoritmo, registro.vertices);
    if (tem_resultado) {
        registro.tem_numero_cores = 1;
        registro.numero_cores = resultado.total_cores;
        valida = coloracao_valida(grafo, &resultado);
        if (!valida) registro.status = MEDICAO_STATUS_ERRO;
    }

    printf("Algoritmo: %s\n", registro.algoritmo);
    printf("Vertices: %zu\n", registro.vertices);
    printf("Arestas: %zu\n", registro.arestas);
    printf("Grau maximo: %zu\n", interferencia.grau_maximo);
    printf("Tempo ms: %.6f\n", registro.tempo_ms);
    printf("Memoria grafo bytes: %zu\n", registro.memoria_grafo_bytes);
    printf("Memoria auxiliar bytes: %zu\n", registro.memoria_auxiliar_bytes);
    if (registro.tem_numero_cores) printf("Cores: %zu\n", registro.numero_cores);
    else puts("Cores: ");
    printf("Valida: %s\n", tem_resultado ? (valida ? "sim" : "nao") : "");
    printf("Status: %s\n", medicao_nome_status(registro.status));
    if (opcoes.algoritmo == ALGORITMO_EXATA) {
        printf("Limite inferior: %zu\n", info.limite_inferior);
        printf("Nos visitados: %llu\n", info.nos_visitados);
    }

    if (opcoes.saida != NULL && !medicao_gravar_csv(opcoes.saida, &registro))
        fprintf(stderr, "Erro ao gravar o registro em %s.\n", opcoes.saida);

    coloracao_liberar(&resultado);
    grafo_destruir(grafo);
    return registro.status == MEDICAO_STATUS_ERRO ? 1 : 0;
}
