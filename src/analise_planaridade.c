#include "analise_planaridade.h"

void analise_planaridade_exibir(FILE *saida, size_t vertices, size_t arestas,
    ResultadoEuler euler, int possui_cruzamentos,
    const size_t *quantidade_cruzamentos)
{
    fprintf(saida, "\nAnalise consolidada de planaridade\nVertices: %zu\nArestas: %zu\n",
        vertices, arestas);
    switch (euler) {
        case EULER_NAO_PLANAR:
            fputs("Condicao de Euler: E <= 3V - 6 nao atendida; grafo nao planar.\n", saida);
            break;
        case EULER_INCONCLUSIVO:
            fputs("Condicao de Euler: E <= 3V - 6 atendida; planaridade inconclusiva.\n", saida);
            break;
        default:
            fputs("Condicao de Euler: nao aplicavel (requer grafo simples com V >= 3).\n", saida);
            break;
    }
    if (quantidade_cruzamentos != NULL) {
        fprintf(saida, "Quantidade de cruzamentos: %zu\n", *quantidade_cruzamentos);
        possui_cruzamentos = *quantidade_cruzamentos > 0U;
    } else {
        fputs("Quantidade de cruzamentos: indisponivel (pendente da #16).\n", saida);
        fputs("Status: consolidacao parcial; contagem ainda nao integrada.\n", saida);
    }
    fputs("Conclusao: ", saida);
    if (euler == EULER_NAO_PLANAR) {
        fputs("a rede nao admite uma representacao plana sem cruzamentos. "
              "Reavaliar as conexoes ou prever isolamento nos cruzamentos.\n", saida);
    }
    if (possui_cruzamentos) {
        fputs("Foram detectados cruzamentos no tracado dos cabos; "
              "avaliar isolamento ou alteracao das rotas.\n", saida);
    } else {
        fputs("O detector atual nao encontrou cruzamentos no tracado analisado. "
              "Esse resultado nao certifica a viabilidade fisica da rede.\n", saida);
    }
    fputs("Limitacoes: a condicao de Euler, isoladamente, nao constitui um teste "
          "completo de planaridade. Cruzamentos no desenho atual nao provam, "
          "por si so, que o grafo seja nao planar.\n", saida);
}
