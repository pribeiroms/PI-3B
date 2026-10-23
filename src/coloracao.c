#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "coloracao.h"

#define SEM_COR SIZE_MAX

typedef struct {
    size_t vertice;
    size_t grau;
} VerticeGrau;

/* Marca as cores dos vizinhos ja coloridos de v. Em vez de limpar o vetor a
 * cada vertice, grava carimbo (passo + 1): so vale a marca do passo atual.
 */
static void marcar_cores_vizinhos(const Grafo *grafo, size_t v,
    const size_t *cores, size_t *marca, size_t carimbo)
{
    const NoAdjacencia *vizinho;
    size_t u, quantidade = grafo_quantidade_vertices(grafo);

    if (grafo_estrutura(grafo) == GRAFO_MATRIZ_ADJACENCIA) {
        for (u = 0U; u < quantidade; ++u)
            if (cores[u] != SEM_COR && grafo_sao_adjacentes(grafo, v, u))
                marca[cores[u]] = carimbo;
        return;
    }
    for (vizinho = grafo_vizinhos(grafo, v); vizinho != NULL; vizinho = vizinho->proximo)
        if (cores[vizinho->vertice] != SEM_COR) marca[cores[vizinho->vertice]] = carimbo;
}

static int comparar_grau_decrescente(const void *a, const void *b)
{
    const VerticeGrau *x = a, *y = b;
    if (x->grau != y->grau) return x->grau > y->grau ? -1 : 1;
    return x->vertice < y->vertice ? -1 : (x->vertice > y->vertice);
}

int coloracao_calcular_graus(const Grafo *grafo, size_t *graus)
{
    size_t indice, origem, destino;

    if (grafo == NULL || (graus == NULL && grafo_quantidade_vertices(grafo) > 0U)) return 0;
    for (indice = 0U; indice < grafo_quantidade_vertices(grafo); ++indice) graus[indice] = 0U;
    for (indice = 0U; indice < grafo_quantidade_arestas(grafo); ++indice) {
        if (!grafo_obter_aresta(grafo, indice, &origem, &destino)) return 0;
        ++graus[origem];
        ++graus[destino];
    }
    return 1;
}

int coloracao_ordem_welsh_powell(const Grafo *grafo, size_t *ordem)
{
    size_t quantidade, v;
    size_t *graus;
    VerticeGrau *pares;

    if (grafo == NULL) return 0;
    quantidade = grafo_quantidade_vertices(grafo);
    if (quantidade == 0U) return 1;
    if (ordem == NULL || quantidade > SIZE_MAX / sizeof(VerticeGrau)) return 0;

    graus = malloc(quantidade * sizeof(size_t));
    pares = malloc(quantidade * sizeof(VerticeGrau));
    if (graus == NULL || pares == NULL || !coloracao_calcular_graus(grafo, graus)) {
        free(graus);
        free(pares);
        return 0;
    }
    for (v = 0U; v < quantidade; ++v) {
        pares[v].vertice = v;
        pares[v].grau = graus[v];
    }
    qsort(pares, quantidade, sizeof(VerticeGrau), comparar_grau_decrescente);
    for (v = 0U; v < quantidade; ++v) ordem[v] = pares[v].vertice;
    free(graus);
    free(pares);
    return 1;
}

int coloracao_gulosa_ordem(const Grafo *grafo, OrdemColoracao ordem,
    ResultadoColoracao *resultado)
{
    size_t quantidade, passo, v, cor;
    size_t *marca, *sequencia;
    int ok;

    if (grafo == NULL || resultado == NULL ||
        (ordem != COLORACAO_ORDEM_NATURAL && ordem != COLORACAO_WELSH_POWELL)) return 0;
    resultado->cores = NULL;
    resultado->quantidade_vertices = 0U;
    resultado->total_cores = 0U;
    quantidade = grafo_quantidade_vertices(grafo);
    if (quantidade == 0U) return 1;
    if (quantidade > SIZE_MAX / sizeof(size_t)) return 0;

    resultado->cores = malloc(quantidade * sizeof(size_t));
    /* Um vertice nunca precisa de mais de V cores (0..V-1). */
    marca = calloc(quantidade, sizeof(size_t));
    sequencia = malloc(quantidade * sizeof(size_t));
    ok = resultado->cores != NULL && marca != NULL && sequencia != NULL;
    if (ok && ordem == COLORACAO_WELSH_POWELL)
        ok = coloracao_ordem_welsh_powell(grafo, sequencia);
    else if (ok)
        for (v = 0U; v < quantidade; ++v) sequencia[v] = v;
    if (!ok) {
        free(resultado->cores);
        free(marca);
        free(sequencia);
        resultado->cores = NULL;
        return 0;
    }

    for (v = 0U; v < quantidade; ++v) resultado->cores[v] = SEM_COR;
    for (passo = 0U; passo < quantidade; ++passo) {
        v = sequencia[passo];
        marcar_cores_vizinhos(grafo, v, resultado->cores, marca, passo + 1U);
        for (cor = 0U; marca[cor] == passo + 1U; ++cor) {}
        resultado->cores[v] = cor;
        if (cor + 1U > resultado->total_cores) resultado->total_cores = cor + 1U;
    }
    resultado->quantidade_vertices = quantidade;
    free(marca);
    free(sequencia);
    return 1;
}

int coloracao_gulosa(const Grafo *grafo, ResultadoColoracao *resultado)
{
    return coloracao_gulosa_ordem(grafo, COLORACAO_ORDEM_NATURAL, resultado);
}

int coloracao_comparar_ordens(const Grafo *grafo, ComparacaoColoracao *comparacao)
{
    ResultadoColoracao natural, welsh_powell;

    if (comparacao == NULL) return 0;
    if (!coloracao_gulosa_ordem(grafo, COLORACAO_ORDEM_NATURAL, &natural)) return 0;
    if (!coloracao_gulosa_ordem(grafo, COLORACAO_WELSH_POWELL, &welsh_powell)) {
        coloracao_liberar(&natural);
        return 0;
    }
    comparacao->cores_ordem_natural = natural.total_cores;
    comparacao->cores_welsh_powell = welsh_powell.total_cores;
    coloracao_liberar(&natural);
    coloracao_liberar(&welsh_powell);
    return 1;
}

/* Intervalo, em nos visitados, entre as consultas ao relogio. */
#define INTERVALO_CONSULTA_TEMPO 4096U

typedef struct {
    const Grafo *grafo;
    size_t quantidade;
    size_t k;
    size_t *cores;
    clock_t inicio;
    double max_segundos;
    unsigned long long nos_visitados;
    int tempo_esgotado;
} BuscaExata;

static double segundos_desde(clock_t inicio)
{
    return (double)(clock() - inicio) / CLOCKS_PER_SEC;
}

/* Poda: retorna 1 se algum vizinho ja colorido de v usa a cor. Os vertices
 * sao coloridos na ordem natural, entao os ja coloridos sao os de indice < v.
 */
static int cor_conflita(const BuscaExata *busca, size_t v, size_t cor)
{
    const NoAdjacencia *vizinho;
    size_t u;

    if (grafo_estrutura(busca->grafo) == GRAFO_MATRIZ_ADJACENCIA) {
        for (u = 0U; u < v; ++u)
            if (busca->cores[u] == cor && grafo_sao_adjacentes(busca->grafo, v, u)) return 1;
        return 0;
    }
    for (vizinho = grafo_vizinhos(busca->grafo, v); vizinho != NULL; vizinho = vizinho->proximo)
        if (vizinho->vertice < v && busca->cores[vizinho->vertice] == cor) return 1;
    return 0;
}

/* Tenta colorir os vertices v..V-1 com no maximo k cores. cores_usadas e a
 * quantidade de cores ja usadas em 0..v-1: oferecer ao vertice v apenas as
 * cores 0..cores_usadas evita repetir coloracoes equivalentes (cores trocadas).
 */
static int colorir_a_partir_de(BuscaExata *busca, size_t v, size_t cores_usadas)
{
    size_t cor, limite;

    if (v == busca->quantidade) return 1;
    ++busca->nos_visitados;
    if (busca->max_segundos > 0.0 &&
        busca->nos_visitados % INTERVALO_CONSULTA_TEMPO == 0U &&
        segundos_desde(busca->inicio) > busca->max_segundos) {
        busca->tempo_esgotado = 1;
        return 0;
    }
    limite = cores_usadas < busca->k ? cores_usadas + 1U : busca->k;
    for (cor = 0U; cor < limite; ++cor) {
        if (cor_conflita(busca, v, cor)) continue;
        busca->cores[v] = cor;
        if (colorir_a_partir_de(busca, v + 1U,
                cor == cores_usadas ? cores_usadas + 1U : cores_usadas)) return 1;
        if (busca->tempo_esgotado) return 0;
    }
    busca->cores[v] = SEM_COR;
    return 0;
}

StatusColoracaoExata coloracao_exata(const Grafo *grafo,
    const LimitesColoracaoExata *limites, ResultadoColoracao *resultado,
    InfoColoracaoExata *info)
{
    BuscaExata busca;
    size_t v;
    StatusColoracaoExata status = COLORACAO_EXATA_OK;

    if (info != NULL) {
        info->segundos = 0.0;
        info->limite_inferior = 0U;
        info->nos_visitados = 0U;
    }
    if (grafo == NULL || resultado == NULL) return COLORACAO_EXATA_ERRO;
    resultado->cores = NULL;
    resultado->quantidade_vertices = 0U;
    resultado->total_cores = 0U;

    busca.grafo = grafo;
    busca.quantidade = grafo_quantidade_vertices(grafo);
    busca.max_segundos = limites != NULL ? limites->max_segundos : 0.0;
    busca.nos_visitados = 0U;
    busca.tempo_esgotado = 0;
    busca.inicio = clock();
    if (limites != NULL && limites->max_vertices > 0U &&
        busca.quantidade > limites->max_vertices) return COLORACAO_EXATA_LIMITE_TAMANHO;
    if (busca.quantidade == 0U) return COLORACAO_EXATA_OK;
    if (busca.quantidade > SIZE_MAX / sizeof(size_t)) return COLORACAO_EXATA_ERRO;

    busca.cores = malloc(busca.quantidade * sizeof(size_t));
    if (busca.cores == NULL) return COLORACAO_EXATA_ERRO;
    for (v = 0U; v < busca.quantidade; ++v) busca.cores[v] = SEM_COR;

    /* Com k = V sempre ha solucao, entao o laco termina. */
    for (busca.k = 1U; !colorir_a_partir_de(&busca, 0U, 0U); ++busca.k) {
        if (busca.tempo_esgotado) {
            status = COLORACAO_EXATA_LIMITE_TEMPO;
            break;
        }
    }
    if (info != NULL) {
        info->segundos = segundos_desde(busca.inicio);
        info->limite_inferior = busca.k;
        info->nos_visitados = busca.nos_visitados;
    }
    if (status != COLORACAO_EXATA_OK) {
        free(busca.cores);
        return status;
    }
    resultado->cores = busca.cores;
    resultado->quantidade_vertices = busca.quantidade;
    resultado->total_cores = busca.k;
    return COLORACAO_EXATA_OK;
}

const char *coloracao_mensagem_exata(StatusColoracaoExata status)
{
    switch (status) {
        case COLORACAO_EXATA_OK:
            return "Coloracao exata concluida; numero cromatico determinado.";
        case COLORACAO_EXATA_LIMITE_TAMANHO:
            return "Coloracao exata nao executada: grafo maior que o limite de vertices.";
        case COLORACAO_EXATA_LIMITE_TEMPO:
            return "Coloracao exata interrompida pelo limite de tempo.";
        default:
            return "Erro na coloracao exata.";
    }
}

int coloracao_validar(const Grafo *grafo, const ResultadoColoracao *resultado,
    ValidacaoColoracao *validacao)
{
    size_t indice, origem, destino, *usos;
    size_t quantidade = grafo_quantidade_vertices(grafo);

    if (grafo == NULL || resultado == NULL || validacao == NULL ||
        resultado->quantidade_vertices != quantidade ||
        (quantidade > 0U && resultado->cores == NULL)) return 0;
    validacao->arestas_em_conflito = 0U;
    validacao->primeira_aresta_conflito = 0U;
    validacao->cores_consistentes = 1;
    for (indice = 0U; indice < grafo_quantidade_arestas(grafo); ++indice) {
        if (!grafo_obter_aresta(grafo, indice, &origem, &destino)) return 0;
        if (resultado->cores[origem] != resultado->cores[destino]) continue;
        if (validacao->arestas_em_conflito == 0U)
            validacao->primeira_aresta_conflito = indice;
        ++validacao->arestas_em_conflito;
    }

    /* total_cores precisa ser exatamente o numero de cores distintas usadas. */
    if (resultado->total_cores > quantidade) {
        validacao->cores_consistentes = 0;
    } else if (quantidade > 0U) {
        usos = calloc(resultado->total_cores + 1U, sizeof(size_t));
        if (usos == NULL) return 0;
        for (indice = 0U; indice < quantidade; ++indice) {
            if (resultado->cores[indice] >= resultado->total_cores) {
                validacao->cores_consistentes = 0;
                break;
            }
            ++usos[resultado->cores[indice]];
        }
        for (indice = 0U; validacao->cores_consistentes &&
             indice < resultado->total_cores; ++indice)
            if (usos[indice] == 0U) validacao->cores_consistentes = 0;
        free(usos);
    } else if (resultado->total_cores != 0U) {
        validacao->cores_consistentes = 0;
    }
    validacao->valida = validacao->arestas_em_conflito == 0U &&
        validacao->cores_consistentes;
    return 1;
}

int coloracao_valida(const Grafo *grafo, const ResultadoColoracao *resultado)
{
    ValidacaoColoracao validacao;
    return coloracao_validar(grafo, resultado, &validacao) && validacao.valida;
}

size_t coloracao_mapear_canais(const ResultadoColoracao *resultado,
    PlanoFrequencias plano, CanalFrequencia **canais, int *erro)
{
    size_t cor, v;
    CanalFrequencia *vetor;

    if (erro != NULL) *erro = 1;
    if (canais != NULL) *canais = NULL;
    if (resultado == NULL || canais == NULL || erro == NULL ||
        !(plano.largura_canal_mhz > 0.0) || !(plano.frequencia_inicial_mhz >= 0.0) ||
        resultado->total_cores > resultado->quantidade_vertices ||
        (resultado->quantidade_vertices > 0U && resultado->cores == NULL)) return 0U;
    *erro = 0;
    if (resultado->total_cores == 0U) return 0U;

    vetor = malloc(resultado->total_cores * sizeof(CanalFrequencia));
    if (vetor == NULL) {
        *erro = 1;
        return 0U;
    }
    for (cor = 0U; cor < resultado->total_cores; ++cor) {
        vetor[cor].numero = cor + 1U;
        vetor[cor].inicio_mhz = plano.frequencia_inicial_mhz +
            (double)cor * plano.largura_canal_mhz;
        vetor[cor].fim_mhz = vetor[cor].inicio_mhz + plano.largura_canal_mhz;
        vetor[cor].torres = 0U;
    }
    for (v = 0U; v < resultado->quantidade_vertices; ++v) {
        if (resultado->cores[v] >= resultado->total_cores) {
            free(vetor);
            *erro = 1;
            return 0U;
        }
        ++vetor[resultado->cores[v]].torres;
    }
    *canais = vetor;
    return resultado->total_cores;
}

void coloracao_exibir_canais(FILE *saida, const CanalFrequencia *canais,
    size_t quantidade)
{
    size_t i, torres = 0U;

    fprintf(saida, "\nDistribuicao de canais de radio\nCanais necessarios: %zu\n",
        quantidade);
    for (i = 0U; i < quantidade; ++i) {
        fprintf(saida, "Canal %zu: %.3f - %.3f MHz, %zu torre(s)\n",
            canais[i].numero, canais[i].inicio_mhz, canais[i].fim_mhz, canais[i].torres);
        torres += canais[i].torres;
    }
    fprintf(saida, "Total de torres: %zu\n", torres);
}

void coloracao_liberar(ResultadoColoracao *resultado)
{
    if (resultado == NULL) return;
    free(resultado->cores);
    resultado->cores = NULL;
    resultado->quantidade_vertices = 0U;
    resultado->total_cores = 0U;
}
