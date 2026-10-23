#!/usr/bin/env python3
"""Mede e imprime a contagem de vertices e arestas de cada subconjunto.

A issue #70 pede a contagem de vertices e arestas de cada subconjunto. A
contagem de arestas nao e propriedade do arquivo: depende de qual grafo se
constroi sobre ele. A Fase II tem dois grafos diferentes, com criterios
proprios, entao cada subconjunto rende uma linha com varias colunas.

A tabela e medida executando o proprio programa, nao copiada a mao. Assim ela
nao envelhece em silencio quando algum criterio mudar: basta rodar de novo.

Uso:
    python scripts/tabela_subconjuntos.py                 # os dois conjuntos
    python scripts/tabela_subconjuntos.py --conjunto fase2
"""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
EXECUTAVEL = ROOT / "build" / "grafo.exe"

CONJUNTOS = {
    "fase1": {
        "titulo": "Dados antigos — subconjuntos da Fase I (amostra uniforme do Brasil)",
        "pasta": ROOT / "data" / "subconjuntos",
        "padrao": "opencellid_n{n}.csv",
        "tamanhos": (100, 500, 1000, 5000),
    },
    "fase2": {
        "titulo": "Dados novos — subconjuntos da Fase II (recorte da regiao mais densa)",
        "pasta": ROOT / "data" / "subconjuntos" / "fase2",
        "padrao": "opencellid_denso_n{n}.csv",
        "tamanhos": (10, 20, 30, 40, 100, 500, 1000, 5000),
    },
}

COLUNAS = ("N", "Vertices", "Arestas Fase I", "Cruzamentos Fase I",
           "Arestas cabeamento", "Arestas interferencia", "Grau maximo",
           "Canais (D+1)", "Sem interferencia")


def executar(dataset: Path, *extras: str) -> str:
    if not EXECUTAVEL.exists():
        raise FileNotFoundError(f"Compile antes: {EXECUTAVEL} nao existe.")
    with tempfile.TemporaryDirectory() as temporario:
        descartavel = Path(temporario) / "execucoes.csv"
        comando = [str(EXECUTAVEL), "--dataset", str(dataset), "--limite", "0",
                   "--saida", str(descartavel), *extras]
        # O programa escreve em UTF-8, inclusive o caminho do dataset que ele
        # repete na saida. Sem fixar a codificacao aqui, o Python no Windows
        # decodifica com a pagina de codigo do sistema (cp1252) e quebra no
        # primeiro acento do caminho -- por exemplo numa pasta "Area de
        # Trabalho" acentuada.
        concluido = subprocess.run(comando, capture_output=True, text=True,
                                   encoding="utf-8", errors="replace")
    if concluido.returncode != 0:
        raise RuntimeError(f"{dataset.name}: {concluido.stderr.strip()}")
    return concluido.stdout


def numero(texto: str, rotulo: str) -> int:
    achado = re.search(rotulo + r"[^0-9]*([0-9]+)", texto)
    if achado is None:
        raise RuntimeError(f"Nao encontrei '{rotulo}' na saida do programa.")
    return int(achado.group(1))


def medir(dataset: Path) -> tuple[int, ...]:
    fase1 = executar(dataset)
    cabeamento = executar(dataset, "--modo", "candidato")
    interferencia = executar(dataset, "--modo", "interferencia")
    grau = numero(interferencia, "Grau maximo")
    return (
        numero(interferencia, "Vertices"),
        numero(fase1, "Arestas"),
        numero(fase1, "Quantidade de cruzamentos"),
        numero(cabeamento, "Arestas"),
        numero(interferencia, "Arestas"),
        grau,
        grau + 1,
        numero(interferencia, "Antenas sem interferencia"),
    )


def montar_tabela(nome: str, linhas: list[str]) -> int:
    conjunto = CONJUNTOS[nome]
    linhas.append(f"### {conjunto['titulo']}")
    linhas.append("")
    linhas.append("| " + " | ".join(COLUNAS) + " |")
    linhas.append("|" + "|".join(["---"] * len(COLUNAS)) + "|")
    faltando = []
    for tamanho in conjunto["tamanhos"]:
        dataset = conjunto["pasta"] / conjunto["padrao"].format(n=tamanho)
        if not dataset.exists():
            faltando.append(dataset.name)
            continue
        valores = medir(dataset)
        linhas.append("| " + " | ".join([str(tamanho)] + [f"{v:,}".replace(",", ".")
                                                          for v in valores]) + " |")
    linhas.append("")
    if faltando:
        print("Arquivos ausentes: " + ", ".join(faltando), file=sys.stderr)
        return 1
    return 0


CABECALHO = """# Contagem de vértices e arestas dos subconjuntos

<!-- ARQUIVO GERADO. Não edite à mão: rode `mingw32-make tabela-subconjuntos`. -->

Este arquivo responde ao último item da issue #70. A contagem de arestas não é
propriedade do arquivo CSV: depende de qual grafo se constrói sobre ele. Como a
Fase II tem dois grafos com critérios próprios, cada subconjunto rende uma linha
com várias colunas.

## Como ler as duas tabelas

São **dois conjuntos de dados diferentes**, e os dois continuam no repositório:

- **Dados antigos** — os subconjuntos sorteados uniformemente do Brasil, usados
  na Fase I. Nada neles foi alterado: os arquivos, os hashes e as contagens
  originais seguem intactos e verificados pelos testes.
- **Dados novos** — o recorte da região mais densa do dataset, criado para a
  Fase II porque a amostra uniforme produz grafos de interferência vazios nos
  tamanhos pequenos, o que tornaria trivial a coloração exata.

Cada tabela traz **todas** as colunas, inclusive as combinações cruzadas: os
grafos da Fase II medidos sobre os dados antigos, e a análise da Fase I medida
sobre os dados novos. Ou seja, o título da seção diz de qual **dado** a linha
veio, e o nome da coluna diz qual **análise** produziu o número.

## O que cada coluna é

As colunas de cabeamento e interferência usam os critérios definidos na issue
#65 e documentados em `MODELAGEM_PESOS_FASE2.md`. "Canais (D+1)" é o grau máximo
mais um, que é o limite superior de cores da coloração gulosa — ou seja, o pior
caso de canais de rádio necessários.

As colunas "Arestas Fase I" e "Cruzamentos Fase I" são a análise da Fase I —
conexão pela antena elegível mais próxima, e detecção de cruzamento de arestas.
Na tabela dos dados novos elas são a **Fase I refeita**, e a comparação entre as
duas tabelas mostra por que a conclusão original sobre planaridade era frágil:
na amostra uniforme os cruzamentos são raros porque as antenas estão a centenas
de quilômetros umas das outras; no recorte denso eles aparecem às centenas, com
um número de arestas parecido. A escolha da amostra, e não a rede em si,
determinava a resposta.

Os números são medidos executando o programa sobre cada subconjunto, nunca
copiados à mão, para a tabela não envelhecer em silêncio quando algum critério
mudar.
"""


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--conjunto", choices=("fase1", "fase2", "ambos"),
                        default="ambos", help="qual conjunto de subconjuntos medir")
    parser.add_argument("--saida", metavar="ARQUIVO",
                        help="grava a tabela em ARQUIVO, em Markdown, "
                             "em vez de apenas imprimir")
    args = parser.parse_args()
    nomes = ("fase1", "fase2") if args.conjunto == "ambos" else (args.conjunto,)
    try:
        status = 0
        linhas: list[str] = []
        for nome in nomes:
            status |= montar_tabela(nome, linhas)
        texto = "\n".join(linhas)
        if args.saida:
            destino = Path(args.saida)
            destino.parent.mkdir(parents=True, exist_ok=True)
            destino.write_text(CABECALHO + "\n" + texto, encoding="utf-8")
            print(f"Tabela gravada em {destino}")
        else:
            print(texto)
        return status
    except (OSError, RuntimeError) as erro:
        print(erro, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())