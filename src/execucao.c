#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "execucao.h"

static int ler_limite(const char *texto, size_t *limite)
{
    const char *p;
    char *fim;
    uintmax_t valor;
    if (*texto == '\0') return 0;
    for (p = texto; *p != '\0'; ++p)
        if (*p < '0' || *p > '9') return 0;
    errno = 0;
    valor = strtoumax(texto, &fim, 10);
    if (errno == ERANGE || *fim != '\0' || valor > SIZE_MAX) return 0;
    *limite = (size_t)valor;
    return 1;
}

void execucao_exibir_ajuda(void)
{
    puts("Uso: grafo.exe [dataset.csv [limite]]\n"
         " ou: grafo.exe [--dataset arquivo] [--limite N] "
         "[--estrutura conjunta] [--saida arquivo.csv]\n"
         "Padroes: dataset versionado, limite 1000, saida results/execucoes.csv.\n"
         "Limite 0 carrega todos os registros validos; N limita os vertices.\n"
         "Estrutura conjunta: lista e matriz mantidas simultaneamente.\n"
         "Selecao exclusiva de lista/matriz ainda indisponivel.\n"
         "Nao misture argumentos posicionais com opcoes nomeadas.\n"
         "--help ou --ajuda exibe esta mensagem.");
}

int execucao_ler_opcoes(int argc, char *argv[], OpcoesExecucao *opcoes)
{
    int i;
    unsigned int vistos = 0U;
    opcoes->dataset = "data/opencellid_brasil_filtrado.csv";
    opcoes->limite = 1000U;
    opcoes->saida = "results/execucoes.csv";
    if (argc == 2 && (strcmp(argv[1], "--help") == 0 ||
                      strcmp(argv[1], "--ajuda") == 0)) return 0;
    if (argc > 1 && argv[1][0] != '-') {
        if (argc > 3 || argv[1][0] == '\0' ||
            (argc == 3 && !ler_limite(argv[2], &opcoes->limite))) goto invalido;
        opcoes->dataset = argv[1];
        return 1;
    }
    for (i = 1; i < argc; i += 2) {
        unsigned int opcao;
        const char *valor;
        if (i + 1 >= argc) goto invalido;
        valor = argv[i + 1];
        if (*valor == '\0') goto invalido;
        if (strcmp(argv[i], "--dataset") == 0) {
            opcao = 1U;
            opcoes->dataset = valor;
        } else if (strcmp(argv[i], "--limite") == 0) {
            opcao = 2U;
            if (!ler_limite(valor, &opcoes->limite)) goto invalido;
        } else if (strcmp(argv[i], "--saida") == 0) {
            opcao = 4U;
            opcoes->saida = valor;
        } else if (strcmp(argv[i], "--estrutura") == 0) {
            opcao = 8U;
            if (strcmp(valor, "lista") == 0 || strcmp(valor, "matriz") == 0) {
                fputs("Selecao exclusiva indisponivel: a API atual mantem lista e matriz juntas.\n", stderr);
                return -1;
            }
            if (strcmp(valor, "conjunta") != 0) goto invalido;
        } else goto invalido;
        if ((vistos & opcao) != 0U) goto invalido;
        vistos |= opcao;
    }
    return 1;
invalido:
    fputs("Argumentos invalidos. Consulte --help. Limite deve ser inteiro nao negativo.\n", stderr);
    return -1;
}

static void escrever_campo(FILE *arquivo, const char *texto)
{
    fputc('"', arquivo);
    for (; *texto != '\0'; ++texto) {
        if (*texto == '"') fputc('"', arquivo);
        fputc(*texto, arquivo);
    }
    fputc('"', arquivo);
}

static const char *nome_euler(ResultadoEuler euler)
{
    switch (euler) {
        case EULER_NAO_PLANAR: return "nao_planar";
        case EULER_INCONCLUSIVO: return "inconclusivo";
        default: return "nao_aplicavel";
    }
}

int execucao_salvar(const OpcoesExecucao *opcoes, const ResultadoExecucao *r)
{
    const char *cabecalho = "data_utc,dataset,limite,estrutura,vertices,arestas,"
        "registros_invalidos,euler,possui_cruzamentos,quantidade_cruzamentos,"
        "status_cruzamentos,memoria_bytes,status_memoria,leitura_cpu_ms,"
        "construcao_cpu_ms,euler_cpu_ms,cruzamentos_cpu_ms,status_analise\n";
    char linha[1024];
    char data[32] = "indisponivel";
    time_t agora = time(NULL);
    struct tm *utc = gmtime(&agora);
    FILE *arquivo = fopen(opcoes->saida, "a+");
    int vazio, ok;
    if (arquivo == NULL) return 0;
    rewind(arquivo);
    vazio = fgets(linha, sizeof(linha), arquivo) == NULL;
    if (ferror(arquivo) || (!vazio && strcmp(linha, cabecalho) != 0)) {
        fclose(arquivo);
        return 0;
    }
    if (fseek(arquivo, 0, SEEK_END) != 0) {
        fclose(arquivo);
        return 0;
    }
    if (utc != NULL) (void)strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%SZ", utc);
    if (vazio) fputs(cabecalho, arquivo);
    escrever_campo(arquivo, data);
    fputc(',', arquivo);
    escrever_campo(arquivo, opcoes->dataset);
    fprintf(arquivo, ",%zu,conjunta,%zu,%zu,%zu,%s,%d,%zu,calculado,,pendente_19,"
        "%.3f,%.3f,%.3f,%.3f,parcial\n", opcoes->limite, r->vertices, r->arestas,
        r->carregamento.registros_invalidos, nome_euler(r->euler),
        r->possui_cruzamentos, r->quantidade_cruzamentos, r->leitura_ms,
        r->construcao_ms, r->euler_ms,
        r->cruzamentos_ms);
    ok = !ferror(arquivo);
    if (fclose(arquivo) != 0) ok = 0;
    return ok;
}
