#ifndef MEDICAO_H
#define MEDICAO_H

/* Medicao de desempenho dos algoritmos da Fase II (issue #86, RF03).
 *
 * Esta etapa traz apenas o cronometro. O registro de cada execucao e a
 * gravacao em CSV entram nas etapas seguintes da mesma issue.
 */

/* Cronometro de tempo decorrido (relogio de parede, monotonico).
 *
 * No Windows usa QueryPerformanceCounter; nos demais sistemas, clock_gettime
 * com CLOCK_MONOTONIC. Nos dois a resolucao e bem menor que 1 ms. Ja o
 * clock() tem resolucao de 1 ms (CLOCKS_PER_SEC = 1000 nesta CRT), o que fazia
 * etapas rapidas aparecerem como 0.000 ms nos registros da Fase I.
 *
 * Mede tempo decorrido, nao tempo de CPU: os algoritmos da Fase II sao
 * sequenciais e nao esperam por I/O, entao os dois valores ficam proximos,
 * mas outros processos da maquina podem inflar a medida.
 */
typedef struct {
    unsigned long long marca; /* leitura bruta do relogio; unidade interna */
    int valido;               /* 0 se o relogio nao estava disponivel */
} Cronometro;

/* Marca o instante inicial. Se o relogio do sistema falhar, o cronometro fica
 * invalido e cronometro_decorrido_ms devolve -1.0. Aceita NULL sem fazer nada.
 */
void cronometro_iniciar(Cronometro *cronometro);

/* Milissegundos, com fracao, desde cronometro_iniciar. Pode ser chamada varias
 * vezes sobre o mesmo cronometro. Devolve -1.0 se o cronometro for NULL, se nao
 * tiver sido iniciado com sucesso ou se o relogio falhar: a mesma convencao de
 * "indisponivel" dos tempos da Fase I. O cronometro precisa ter passado por
 * cronometro_iniciar antes.
 */
double cronometro_decorrido_ms(const Cronometro *cronometro);

#endif
