/* clock_gettime exige esta macro em -std=c11; ela precisa vir antes de
 * qualquer #include. No Windows nao e usada. */
#ifndef _WIN32
#define _POSIX_C_SOURCE 199309L
#endif

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "medicao.h"
#include "utf8io.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32

/* Ticks por segundo do contador de desempenho. Nao muda enquanto o sistema
 * esta ligado, entao e lida uma vez. 0 indica que a leitura falhou. */
static unsigned long long frequencia_ticks(void)
{
    static unsigned long long frequencia = 0ULL;
    LARGE_INTEGER lida;

    if (frequencia == 0ULL && QueryPerformanceFrequency(&lida) && lida.QuadPart > 0)
        frequencia = (unsigned long long)lida.QuadPart;
    return frequencia;
}

static int ler_marca(unsigned long long *marca)
{
    LARGE_INTEGER lida;

    if (frequencia_ticks() == 0ULL || !QueryPerformanceCounter(&lida) ||
        lida.QuadPart < 0) return 0;
    *marca = (unsigned long long)lida.QuadPart;
    return 1;
}

static double ticks_para_ms(unsigned long long ticks)
{
    return (double)ticks * 1000.0 / (double)frequencia_ticks();
}

#else

/* A marca e o tempo em nanossegundos desde um instante arbitrario. */
static int ler_marca(unsigned long long *marca)
{
    struct timespec agora;

    if (clock_gettime(CLOCK_MONOTONIC, &agora) != 0) return 0;
    *marca = (unsigned long long)agora.tv_sec * 1000000000ULL +
        (unsigned long long)agora.tv_nsec;
    return 1;
}

static double ticks_para_ms(unsigned long long ticks)
{
    return (double)ticks / 1000000.0;
}

#endif

void cronometro_iniciar(Cronometro *cronometro)
{
    if (cronometro == NULL) return;
    cronometro->marca = 0ULL;
    cronometro->valido = ler_marca(&cronometro->marca);
}

double cronometro_decorrido_ms(const Cronometro *cronometro)
{
    unsigned long long agora = 0ULL;

    if (cronometro == NULL || !cronometro->valido || !ler_marca(&agora) ||
        agora < cronometro->marca) return -1.0;
    return ticks_para_ms(agora - cronometro->marca);
}

int medicao_preencher_do_grafo(RegistroMedicao *registro, const char *algoritmo,
    const Grafo *grafo, size_t limite_n)
{
    RegistroMedicao novo = {0};
    EstimativaMemoriaGrafo memoria;
    EstruturaGrafo estrutura;

    if (registro == NULL || algoritmo == NULL || grafo == NULL) return 0;
    estrutura = grafo_estrutura(grafo);
    if (estrutura != GRAFO_LISTA_ADJACENCIA &&
        estrutura != GRAFO_MATRIZ_ADJACENCIA) return 0;
    if (!grafo_estimar_memoria(grafo, &memoria)) return 0;
    novo.algoritmo = algoritmo;
    novo.estrutura = estrutura;
    novo.limite_n = limite_n;
    novo.vertices = grafo_quantidade_vertices(grafo);
    novo.arestas = grafo_quantidade_arestas(grafo);
    novo.memoria_grafo_bytes = estrutura == GRAFO_LISTA_ADJACENCIA ?
        memoria.memoria_lista_total_bytes : memoria.memoria_matriz_total_bytes;
    novo.status = MEDICAO_STATUS_OK;
    *registro = novo;
    return 1;
}

#define CABECALHO_CSV \
    "data_utc,algoritmo,estrutura,limite_n,vertices,arestas,tempo_ms," \
    "memoria_grafo_bytes,memoria_auxiliar_bytes,custo_agm_km,numero_cores,status\n"

const char *medicao_nome_status(StatusMedicao status)
{
    switch (status) {
        case MEDICAO_STATUS_OK: return "ok";
        case MEDICAO_STATUS_LIMITE_TEMPO: return "limite_tempo";
        case MEDICAO_STATUS_LIMITE_TAMANHO: return "limite_tamanho";
        case MEDICAO_STATUS_ERRO: return "erro";
    }
    return NULL;
}

static const char *nome_estrutura(EstruturaGrafo estrutura)
{
    return estrutura == GRAFO_LISTA_ADJACENCIA ? "lista" : "matriz";
}

static int registro_valido(const RegistroMedicao *registro)
{
    if (registro == NULL || registro->algoritmo == NULL) return 0;
    if (registro->estrutura != GRAFO_LISTA_ADJACENCIA &&
        registro->estrutura != GRAFO_MATRIZ_ADJACENCIA) return 0;
    if (medicao_nome_status(registro->status) == NULL) return 0;
    if (registro->tem_custo_agm &&
        (!isfinite(registro->custo_agm_km) || registro->custo_agm_km < 0.0))
        return 0;
    return 1;
}

/* Texto entre aspas, com aspas internas dobradas, como no CSV da Fase I. */
static void escrever_campo(FILE *arquivo, const char *texto)
{
    fputc('"', arquivo);
    for (; *texto != '\0'; ++texto) {
        if (*texto == '"') fputc('"', arquivo);
        fputc(*texto, arquivo);
    }
    fputc('"', arquivo);
}

int medicao_gravar_csv(const char *caminho, const RegistroMedicao *registro)
{
    char linha[512];
    char data[32] = "indisponivel";
    time_t agora = time(NULL);
    struct tm *utc;
    FILE *arquivo;
    int vazio, ok;

    if (caminho == NULL || !registro_valido(registro)) return 0;
    arquivo = utf8_fopen(caminho, "a+");
    if (arquivo == NULL) return 0;
    rewind(arquivo);
    vazio = fgets(linha, sizeof(linha), arquivo) == NULL;
    if (ferror(arquivo) || (!vazio && strcmp(linha, CABECALHO_CSV) != 0)) {
        fclose(arquivo);
        return 0;
    }
    if (fseek(arquivo, 0, SEEK_END) != 0) {
        fclose(arquivo);
        return 0;
    }
    utc = gmtime(&agora);
    if (utc != NULL) (void)strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%SZ", utc);
    if (vazio) fputs(CABECALHO_CSV, arquivo);
    escrever_campo(arquivo, data);
    fputc(',', arquivo);
    escrever_campo(arquivo, registro->algoritmo);
    fprintf(arquivo, ",%s,%zu,%zu,%zu,", nome_estrutura(registro->estrutura),
        registro->limite_n, registro->vertices, registro->arestas);
    if (isfinite(registro->tempo_ms) && registro->tempo_ms >= 0.0)
        fprintf(arquivo, "%.6f", registro->tempo_ms);
    fprintf(arquivo, ",%zu,%zu,", registro->memoria_grafo_bytes,
        registro->memoria_auxiliar_bytes);
    if (registro->tem_custo_agm) fprintf(arquivo, "%.4f", registro->custo_agm_km);
    fputc(',', arquivo);
    if (registro->tem_numero_cores) fprintf(arquivo, "%zu", registro->numero_cores);
    fprintf(arquivo, ",%s\n", medicao_nome_status(registro->status));
    ok = !ferror(arquivo);
    if (fclose(arquivo) != 0) ok = 0;
    return ok;
}