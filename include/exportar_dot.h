#ifndef EXPORTAR_DOT_H
#define EXPORTAR_DOT_H

#include <stddef.h>

#include "grafo.h"

/* Exporta o grafo em formato Graphviz DOT usando as coordenadas geograficas
 * reais das antenas. Todas as funcoes deste modulo posicionam os vertices pela
 * latitude e longitude; o que muda entre elas e a area enquadrada.
 *
 * Em todas, as arestas envolvidas em cruzamentos e os vertices que sao seus
 * extremos saem destacados em vermelho. Passar cruzamentos == NULL ou
 * quantidade_cruzamentos == 0 exporta sem destaques.
 */

/* O diametro dos vertices e deduzido do comprimento tipico das arestas
 * exibidas, de modo que o vertice fique bem menor que a rota que sai dele e as
 * linhas continuem visiveis quando se amplia a imagem.
 */

/* Mapa da area completa: todas as antenas carregadas, do extremo sul ao
 * extremo norte do recorte do dataset. Retorna 1 em sucesso e 0 em erro.
 */
int exportar_dot(const Grafo *grafo, const Cruzamento *cruzamentos,
    size_t quantidade_cruzamentos, const char *caminho);

/* Serie de recortes com zoom crescente sobre a regiao de maior concentracao.
 *
 * A partir de "results/mapa.dot" sao gravados:
 *   results/mapa_1_regiao.dot    faixa central das coordenadas (descarta as
 *                                antenas distantes, que esticariam o desenho)
 *   results/mapa_2_noroeste.dot  os quatro quadrantes dessa faixa, cada um
 *   results/mapa_3_nordeste.dot  ocupando a area inteira do desenho
 *   results/mapa_4_sudoeste.dot
 *   results/mapa_5_sudeste.dot
 *
 * Quadrantes sem antenas sao ignorados. O tamanho dos vertices e ajustado a
 * quantidade exibida, de modo que cada recorte fique legivel.
 *
 * Retorna quantos arquivos foram gravados; 0 indica erro.
 */
size_t exportar_dot_mapas(const Grafo *grafo, const Cruzamento *cruzamentos,
    size_t quantidade_cruzamentos, const char *caminho);

#endif