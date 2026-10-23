#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "grafo.h"

/* Tolerancia de 1 metro. Os valores esperados foram calculados pela formula de
 * Haversine com raio terrestre de 6.371 km, o mesmo adotado pelo projeto. */
#define TOLERANCIA_KM 0.001

static CoordenadaGeografica em(double latitude, double longitude)
{
    CoordenadaGeografica coordenada;
    coordenada.latitude = latitude;
    coordenada.longitude = longitude;
    return coordenada;
}

static void verificar(const char *caso, CoordenadaGeografica a,
    CoordenadaGeografica b, double esperado_km)
{
    double obtido = grafo_distancia_km(a, b);
    if (fabs(obtido - esperado_km) > TOLERANCIA_KM) {
        printf("Falhou: %s. Esperado %.4f km, obtido %.4f km.\n", caso,
            esperado_km, obtido);
        assert(0);
    }
}

static void teste_pares_de_distancia_conhecida(void)
{
    /* Coordenadas iguais nao tem distancia. */
    verificar("mesmo ponto", em(0.0, 0.0), em(0.0, 0.0), 0.0);
    verificar("mesmo ponto fora da origem", em(-22.9068, -43.1729),
        em(-22.9068, -43.1729), 0.0);

    /* Um grau de latitude vale o mesmo em qualquer lugar do globo. */
    verificar("1 grau de latitude no equador", em(0.0, 0.0), em(1.0, 0.0),
        111.1949);
    verificar("1 grau de latitude em latitude alta", em(60.0, 10.0),
        em(61.0, 10.0), 111.1949);

    /* Um grau de longitude encurta conforme se afasta do equador: em 60 graus
     * de latitude vale metade do valor no equador (cos 60 = 0,5). */
    verificar("1 grau de longitude no equador", em(0.0, 0.0), em(0.0, 1.0),
        111.1949);
    verificar("1 grau de longitude em latitude 60", em(60.0, 0.0),
        em(60.0, 1.0), 55.5969);

    /* Casos extremos do globo. */
    verificar("quarto de volta pelo equador", em(0.0, 0.0), em(0.0, 90.0),
        10007.5434);
    verificar("antipodas pelo equador", em(0.0, 0.0), em(0.0, 180.0),
        20015.0868);
    verificar("polo sul ao polo norte", em(-90.0, 0.0), em(90.0, 0.0),
        20015.0868);

    /* Pares reais, conferidos contra o valor publicado das capitais. */
    verificar("Rio de Janeiro a Sao Paulo", em(-22.9068, -43.1729),
        em(-23.5505, -46.6333), 360.7488);
    verificar("Rio de Janeiro a Brasilia", em(-22.9068, -43.1729),
        em(-15.7939, -47.8828), 932.3484);
}

static void teste_simetria(void)
{
    CoordenadaGeografica a = em(-22.9068, -43.1729);
    CoordenadaGeografica b = em(-15.7939, -47.8828);
    /* Igualdade exata entre doubles depende do compilador e da libm: a ordem
     * dos argumentos muda a ultima casa decimal. 1e-9 km e um micrometro. */
    assert(fabs(grafo_distancia_km(a, b) - grafo_distancia_km(b, a)) < 1e-9);
}

static void teste_distancias_curtas(void)
{
    /* A ordem de grandeza das arestas do dataset e de centenas de metros.
     * Em torno de 23 graus sul, 0,001 grau de latitude vale cerca de 111 m. */
    double curta = grafo_distancia_km(em(-22.9000, -43.2000),
        em(-22.9010, -43.2000));
    assert(curta > 0.110 && curta < 0.112);

    /* Deslocamentos minusculos continuam positivos e nao viram zero. */
    assert(grafo_distancia_km(em(-22.9, -43.2), em(-22.9, -43.200001)) > 0.0);
}

static void teste_cruzamento_do_meridiano_e_do_equador(void)
{
    /* Sinais opostos de longitude nao devem produzir distancia negativa nem
     * somar a volta completa. */
    double sobre_meridiano = grafo_distancia_km(em(0.0, -0.5), em(0.0, 0.5));
    double sobre_equador = grafo_distancia_km(em(-0.5, 0.0), em(0.5, 0.0));
    assert(fabs(sobre_meridiano - 111.1949) < TOLERANCIA_KM);
    assert(fabs(sobre_equador - 111.1949) < TOLERANCIA_KM);
}

int main(void)
{
    teste_pares_de_distancia_conhecida();
    teste_simetria();
    teste_distancias_curtas();
    teste_cruzamento_do_meridiano_e_do_equador();
    puts("Testes de distancia geografica passaram.");
    return 0;
}