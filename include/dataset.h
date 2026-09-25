#ifndef DATASET_H
#define DATASET_H

#include <stddef.h>

#include "grafo.h"

typedef struct {
    size_t linhas_lidas;
    size_t registros_carregados;
    size_t registros_invalidos;
} RelatorioCarregamento;

/*
 * Carrega registros do CSV OpenCelliD no grafo.
 *
 * limite igual a 0 carrega todos os registros validos. Registros malformados
 * ou com coordenadas/valores fora do dominio sao ignorados e contabilizados
 * em registros_invalidos. Retorna 1 em sucesso, inclusive quando houver
 * registros invalidos; retorna 0 para falhas de abertura, leitura ou cabecalho.
 */
int dataset_carregar_opencellid(Grafo *grafo, const char *caminho, size_t limite,
                                RelatorioCarregamento *relatorio, char *erro,
                                size_t tamanho_erro);

#endif
