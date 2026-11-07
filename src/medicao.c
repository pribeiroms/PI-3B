/* clock_gettime exige esta macro em -std=c11; ela precisa vir antes de
 * qualquer #include. No Windows nao e usada. */
#ifndef _WIN32
#define _POSIX_C_SOURCE 199309L
#endif

#include <stddef.h>

#include "medicao.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
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
