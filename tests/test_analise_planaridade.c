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
    analise_planaridade_exibir(saida, 5U, 10U, euler, possui, quantidade);
    rewind(saida);
    lidos = fread(texto, 1U, sizeof(texto) - 1U, saida);
    assert(!ferror(saida));
    texto[lidos] = '\0';
    assert(strstr(texto, "Vertices: 5\nArestas: 10\n") != NULL);
    assert(strstr(texto, condicao) != NULL);
    assert(strstr(texto, contagem) != NULL);
    assert(strstr(texto, conclusao) != NULL);
    assert(strstr(texto, "nao constitui um teste completo de planaridade") != NULL);
    if (quantidade == NULL) {
        assert(strstr(texto, "consolidacao parcial") != NULL);
        assert(strstr(texto, "Quantidade de cruzamentos: 0") == NULL);
        assert(strstr(texto, "Quantidade de cruzamentos: 1") == NULL);
    }
    fclose(saida);
    assert(remove("build/test_relatorio_planaridade.tmp") == 0);
}

int main(void)
{
    size_t zero = 0U, tres = 3U;
    verificar(EULER_INCONCLUSIVO, 1, NULL, "planaridade inconclusiva",
        "pendente da #16", "avaliar isolamento ou alteracao das rotas");
    verificar(EULER_INCONCLUSIVO, 0, NULL, "planaridade inconclusiva",
        "pendente da #16", "detector atual nao encontrou");
    verificar(EULER_NAO_PLANAR, 1, &tres, "nao atendida",
        "Quantidade de cruzamentos: 3", "rede nao admite uma representacao plana");
    verificar(EULER_NAO_APLICAVEL, 0, &zero, "nao aplicavel",
        "Quantidade de cruzamentos: 0", "detector atual nao encontrou");
    puts("Testes do relatorio de planaridade passaram.");
    return 0;
}
