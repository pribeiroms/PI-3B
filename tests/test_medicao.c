#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "medicao.h"

#define AMOSTRAS 10000U

static void teste_leitura_nao_negativa_e_finita(void)
{
    Cronometro cronometro;
    double tempo;

    cronometro_iniciar(&cronometro);
    tempo = cronometro_decorrido_ms(&cronometro);
    assert(isfinite(tempo));
    assert(tempo >= 0.0);
}

static void teste_leituras_nao_decrescem(void)
{
    Cronometro cronometro;
    double anterior = 0.0;
    unsigned int i;

    cronometro_iniciar(&cronometro);
    for (i = 0U; i < AMOSTRAS; ++i) {
        double atual = cronometro_decorrido_ms(&cronometro);
        assert(atual >= anterior);
        anterior = atual;
    }
}

/* Consulta o cronometro em laco ate ele marcar 5 ms. O limite de iteracoes
 * impede que um relogio quebrado, que devolve -1.0 sempre, prenda o teste para
 * sempre. */
static void teste_mede_intervalo_de_milissegundos(void)
{
    Cronometro cronometro;
    unsigned long long i;
    double tempo = 0.0;

    cronometro_iniciar(&cronometro);
    for (i = 0ULL; i < 50000000ULL && tempo < 5.0; ++i)
        tempo = cronometro_decorrido_ms(&cronometro);
    assert(tempo >= 5.0);
}

/* O motivo de existir do cronometro: distinguir instantes separados por menos
 * de 1 ms, onde clock() devolveria 0.000. Usa o menor passo positivo entre
 * leituras seguidas, que nao e afetado por uma preempcao ocasional. */
static void teste_resolucao_menor_que_um_milissegundo(void)
{
    Cronometro cronometro;
    double anterior;
    double menor_passo = 1.0e9;
    unsigned int i;

    cronometro_iniciar(&cronometro);
    anterior = cronometro_decorrido_ms(&cronometro);
    for (i = 0U; i < AMOSTRAS; ++i) {
        double atual = cronometro_decorrido_ms(&cronometro);
        double passo = atual - anterior;
        if (passo > 0.0 && passo < menor_passo) menor_passo = passo;
        anterior = atual;
    }
    assert(menor_passo < 1.0);
}

static void teste_entradas_invalidas(void)
{
    Cronometro invalido;

    cronometro_iniciar(NULL);
    assert(cronometro_decorrido_ms(NULL) == -1.0);
    invalido.marca = 0ULL;
    invalido.valido = 0;
    assert(cronometro_decorrido_ms(&invalido) == -1.0);
}

int main(void)
{
    teste_leitura_nao_negativa_e_finita();
    teste_leituras_nao_decrescem();
    teste_mede_intervalo_de_milissegundos();
    teste_resolucao_menor_que_um_milissegundo();
    teste_entradas_invalidas();
    puts("Testes do cronometro passaram.");
    return 0;
}
