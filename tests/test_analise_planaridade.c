#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "analise_planaridade.h"

static void verificar(ResultadoEuler euler, int possui, const size_t *quantidade,
    const char *condicao, const char *contagem, const char *conclusao)
{
    char texto[4096];
    size_t lidos;
    FILE *saida = fopen("build/test_relatorio_planaridade.tmp", "w+b");
    assert(saida != NULL);
    analise_planaridade_exibir(saida, 5U, 10U, euler, possui, quantidade, NULL);
    rewind(saida);
    lidos = fread(texto, 1U, sizeof(texto) - 1U, saida);
    assert(!ferror(saida));
    texto[lidos] = '\0';
    assert(strstr(texto, "Vertices: 5\nArestas: 10\n") != NULL);
    assert(strstr(texto, condicao) != NULL);
    assert(strstr(texto, contagem) != NULL);
    assert(strstr(texto, conclusao) != NULL);
    assert(strstr(texto, "nao constitui um teste completo de planaridade") != NULL);
    assert(strstr(texto, "consolidacao parcial") == NULL);
    /* Sem detalhamento informado, a secao nao aparece. */
    assert(strstr(texto, "Detalhamento dos cruzamentos") == NULL);
    fclose(saida);
    assert(remove("build/test_relatorio_planaridade.tmp") == 0);
}

/* O detalhamento, quando informado, sai logo apos a quantidade e antes da
 * conclusao. */
static void verificar_detalhamento(void)
{
    const char *detalhamento =
        "  Aresta 21 (antenas 76-77) x Aresta 62 (antenas 140-136)\n";
    char texto[4096];
    size_t lidos, posicao_quantidade, posicao_detalhe, posicao_conclusao;
    size_t tres = 3U;
    FILE *saida = fopen("build/test_relatorio_detalhe.tmp", "w+b");

    assert(saida != NULL);
    analise_planaridade_exibir(saida, 5U, 10U, EULER_INCONCLUSIVO, 1, &tres,
        detalhamento);
    rewind(saida);
    lidos = fread(texto, 1U, sizeof(texto) - 1U, saida);
    assert(!ferror(saida));
    texto[lidos] = '\0';
    assert(strstr(texto, "Detalhamento dos cruzamentos:") != NULL);
    assert(strstr(texto, "Aresta 21 (antenas 76-77)") != NULL);
    posicao_quantidade = (size_t)(strstr(texto, "Quantidade de cruzamentos") - texto);
    posicao_detalhe = (size_t)(strstr(texto, "Detalhamento dos cruzamentos") - texto);
    posicao_conclusao = (size_t)(strstr(texto, "Conclusao:") - texto);
    assert(posicao_quantidade < posicao_detalhe);
    assert(posicao_detalhe < posicao_conclusao);
    fclose(saida);
    assert(remove("build/test_relatorio_detalhe.tmp") == 0);
}

int main(void)
{
    size_t zero = 0U, tres = 3U;
    verificar(EULER_INCONCLUSIVO, 1, &tres, "planaridade inconclusiva",
        "Quantidade de cruzamentos: 3", "avaliar isolamento ou alteracao das rotas");
    verificar(EULER_INCONCLUSIVO, 0, &zero, "planaridade inconclusiva",
        "Quantidade de cruzamentos: 0", "detector atual nao encontrou");
    verificar(EULER_NAO_PLANAR, 1, &tres, "nao atendida",
        "Quantidade de cruzamentos: 3", "rede nao admite uma representacao plana");
    verificar(EULER_NAO_APLICAVEL, 0, &zero, "nao aplicavel",
        "Quantidade de cruzamentos: 0", "detector atual nao encontrou");
    verificar_detalhamento();
    puts("Testes do relatorio de planaridade passaram.");
    return 0;
}