#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dataset.h"

#define TAMANHO_LINHA_CSV 512U
#define CAMPOS_OPENCELLID 9U

static void definir_erro(char *erro, size_t tamanho_erro, const char *mensagem)
{
    if (erro != NULL && tamanho_erro > 0U) snprintf(erro, tamanho_erro, "%s", mensagem);
}

static int converter_uint(const char *texto, unsigned int *valor)
{
    char *fim;
    unsigned long convertido;

    if (texto == NULL || *texto == '\0' || texto[0] == '-') return 0;
    errno = 0;
    convertido = strtoul(texto, &fim, 10);
    if (errno == ERANGE || *fim != '\0' || convertido > UINT_MAX) return 0;
    *valor = (unsigned int)convertido;
    return 1;
}

static int converter_double(const char *texto, double *valor)
{
    char *fim;

    if (texto == NULL || *texto == '\0') return 0;
    errno = 0;
    *valor = strtod(texto, &fim);
    return errno != ERANGE && *fim == '\0';
}

static int separar_campos(char *linha, char **campos)
{
    size_t indice;
    char *inicio = linha;

    for (indice = 0U; indice < CAMPOS_OPENCELLID; ++indice) {
        char *virgula = strchr(inicio, ',');
        campos[indice] = inicio;
        if (indice + 1U == CAMPOS_OPENCELLID) return virgula == NULL;
        if (virgula == NULL) return 0;
        *virgula = '\0';
        inicio = virgula + 1;
    }
    return 0;
}

static int carregar_registro(Grafo *grafo, char *linha)
{
    char *campos[CAMPOS_OPENCELLID];
    unsigned int mcc, net, area, cell, amostras;
    double latitude, longitude, alcance;

    linha[strcspn(linha, "\r\n")] = '\0';
    if (!separar_campos(linha, campos) || campos[0][0] == '\0' ||
        !converter_uint(campos[1], &mcc) || !converter_uint(campos[2], &net) ||
        !converter_uint(campos[3], &area) || !converter_uint(campos[4], &cell) ||
        !converter_double(campos[5], &latitude) || !converter_double(campos[6], &longitude) ||
        !converter_double(campos[7], &alcance) || !converter_uint(campos[8], &amostras)) return 0;
    (void)amostras;
    return grafo_adicionar_antena(grafo, mcc, net, area, cell, latitude, longitude, alcance) >= 0;
}

int dataset_carregar_opencellid(Grafo *grafo, const char *caminho, size_t limite,
                                RelatorioCarregamento *relatorio, char *erro,
                                size_t tamanho_erro)
{
    const char cabecalho[] = "radio,mcc,net,area,cell,lat,lon,range,samples";
    char linha[TAMANHO_LINHA_CSV];
    FILE *arquivo;
    RelatorioCarregamento relatorio_local;

    if (relatorio == NULL) relatorio = &relatorio_local;
    memset(relatorio, 0, sizeof(*relatorio));
    definir_erro(erro, tamanho_erro, "");
    if (grafo == NULL || caminho == NULL) {
        definir_erro(erro, tamanho_erro, "Grafo ou caminho invalido.");
        return 0;
    }
    arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        definir_erro(erro, tamanho_erro, "Nao foi possivel abrir o dataset.");
        return 0;
    }
    if (fgets(linha, sizeof(linha), arquivo) == NULL) {
        definir_erro(erro, tamanho_erro, "Cabecalho CSV invalido.");
        fclose(arquivo);
        return 0;
    }
    linha[strcspn(linha, "\r\n")] = '\0';
    if (strcmp(linha, cabecalho) != 0) {
        definir_erro(erro, tamanho_erro, "Cabecalho CSV invalido.");
        fclose(arquivo);
        return 0;
    }
    while ((limite == 0U || relatorio->registros_carregados < limite) &&
           fgets(linha, sizeof(linha), arquivo) != NULL) {
        ++relatorio->linhas_lidas;
        if (strchr(linha, '\n') == NULL && !feof(arquivo)) {
            int caractere;
            while ((caractere = fgetc(arquivo)) != '\n' && caractere != EOF) {}
            ++relatorio->registros_invalidos;
        } else if (carregar_registro(grafo, linha)) {
            ++relatorio->registros_carregados;
        } else {
            ++relatorio->registros_invalidos;
        }
    }
    if (ferror(arquivo)) {
        definir_erro(erro, tamanho_erro, "Erro durante a leitura do dataset.");
        fclose(arquivo);
        return 0;
    }
    fclose(arquivo);
    return 1;
}
