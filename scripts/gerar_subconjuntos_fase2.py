#!/usr/bin/env python3
"""Gera subconjuntos geograficamente locais e reproduziveis para a Fase II.

Os subconjuntos da Fase I (scripts/gerar_subconjuntos.py) sorteiam antenas
uniformemente do Brasil inteiro. Isso serve para a analise de planaridade, mas
nao para a Fase II: antenas sorteadas assim ficam a centenas de quilometros
umas das outras e quase nunca satisfazem o criterio de interferencia, que exige
distancia menor ou igual a soma dos alcances -- tipicamente 2 km. Medido no
dataset, N=10, 20, 30 e 40 uniformes produzem grafos de interferencia com ZERO
arestas, o que tornaria trivial a coloracao exata.

Este gerador recorta uma regiao densa em vez de sortear:

  1. divide o territorio numa grade de celulas de 0,1 grau;
  2. escolhe a celula com mais antenas (empate pela menor latitude e depois
     pela menor longitude);
  3. toma as N antenas mais proximas do centro dessa celula.

Nao ha sorteio nem semente: dado o mesmo CSV de origem, o resultado e sempre o
mesmo. Os subconjuntos sao aninhados por construcao, porque "as N mais
proximas" contem "as N-1 mais proximas".

Os arquivos da Fase I nao sao tocados: estes vao para data/subconjuntos/fase2/.
"""

from __future__ import annotations

import argparse
import hashlib
import math
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "data" / "opencellid_brasil_filtrado.csv"
DESTINATION = ROOT / "data" / "subconjuntos" / "fase2"
SIZES = (10, 20, 30, 40, 100, 500, 1000, 5000)
HEADER = b"radio,mcc,net,area,cell,lat,lon,range,samples"
SOURCE_SHA256 = "5EB50BD6954466F08ECA483B2D9DA18B50C297E54710532FB7D0646B2987F8DB"

# Lado da celula da grade, em graus. Com 0,1 grau a celula tem cerca de 11 km
# de lado, o tamanho de uma regiao metropolitana densa.
CELULA_GRAUS = 0.1

RAIO_TERRA_KM = 6371.0


def distancia_km(origem: tuple[float, float], destino: tuple[float, float]) -> float:
    """Haversine, o mesmo criterio de grafo_distancia_km no codigo C."""
    lat1, lon1 = map(math.radians, origem)
    lat2, lon2 = map(math.radians, destino)
    seno = (math.sin((lat2 - lat1) / 2) ** 2
            + math.cos(lat1) * math.cos(lat2) * math.sin((lon2 - lon1) / 2) ** 2)
    return 2 * RAIO_TERRA_KM * math.asin(math.sqrt(seno))


def is_valid(raw: bytes) -> bool:
    """Replica a validacao do carregador em C (src/dataset.c).

    O campo cell usa 64 bits de proposito: o identificador do 5G NR tem 36
    bits. Aceitar apenas 32 descartava 84% das antenas NR do dataset.
    """
    body = raw.rstrip(b"\r\n")
    if len(body) > 510 or b'"' in body:
        return False
    try:
        fields = body.decode("ascii").split(",")
        if len(fields) != 9 or not fields[0] or len(fields[0]) > 15:
            return False
        for value in fields[1:4] + [fields[8]]:
            if not value.isdecimal() or int(value) > 0xFFFFFFFF:
                return False
        if not fields[4].isdecimal() or int(fields[4]) > 0xFFFFFFFFFFFFFFFF:
            return False
        latitude, longitude, reach = map(float, fields[5:8])
    except (UnicodeDecodeError, ValueError, OverflowError):
        return False
    return (
        -90 <= latitude <= 90
        and -180 <= longitude <= 180
        and reach >= 0
        and all(map(lambda number: number == number and abs(number) != float("inf"),
                    (latitude, longitude, reach)))
    )


def celula_mais_densa(registros: list[tuple[float, float, int, bytes]]) -> tuple[int, int, int]:
    """Indices da celula com mais antenas, desempatando pela menor coordenada."""
    contagem: dict[tuple[int, int], int] = {}
    for latitude, longitude, _, _ in registros:
        chave = (math.floor(latitude / CELULA_GRAUS),
                 math.floor(longitude / CELULA_GRAUS))
        contagem[chave] = contagem.get(chave, 0) + 1
    (lat_idx, lon_idx), quantidade = min(
        contagem.items(), key=lambda item: (-item[1], item[0][0], item[0][1]))
    return lat_idx, lon_idx, quantidade


def expected_outputs() -> tuple[dict[str, bytes], str, dict[str, object]]:
    source_bytes = SOURCE.read_bytes()
    source_hash = hashlib.sha256(source_bytes).hexdigest().upper()
    if source_hash != SOURCE_SHA256:
        raise ValueError("Hash do dataset fonte mudou; revise o protocolo antes de regenerar.")
    lines = source_bytes.splitlines(keepends=True)
    if not lines or lines[0].rstrip(b"\r\n") != HEADER:
        raise ValueError("Cabecalho inesperado no dataset fonte.")

    registros: list[tuple[float, float, int, bytes]] = []
    for index, raw in enumerate(lines[1:], start=1):
        if not is_valid(raw):
            continue
        campos = raw.rstrip(b"\r\n").decode("ascii").split(",")
        registros.append((float(campos[5]), float(campos[6]), index, raw))

    if len(registros) < max(SIZES):
        raise ValueError(f"Dataset possui apenas {len(registros)} registros validos.")

    lat_idx, lon_idx, densidade = celula_mais_densa(registros)
    centro = ((lat_idx + 0.5) * CELULA_GRAUS, (lon_idx + 0.5) * CELULA_GRAUS)

    # O indice da linha desempata distancias iguais, o que torna a ordem total
    # e o resultado identico em qualquer maquina.
    ordenados = sorted(registros,
                       key=lambda item: (distancia_km((item[0], item[1]), centro), item[2]))

    outputs: dict[str, bytes] = {}
    raios: dict[int, float] = {}
    for size in SIZES:
        selecionados = sorted(ordenados[:size], key=lambda item: item[2])
        output = DESTINATION / f"opencellid_denso_n{size}.csv"
        outputs[str(output)] = lines[0] + b"".join(item[3] for item in selecionados)
        ultimo = ordenados[size - 1]
        raios[size] = distancia_km((ultimo[0], ultimo[1]), centro)

    contexto = {
        "centro": centro,
        "densidade": densidade,
        "validos": len(registros),
        "raios": raios,
    }
    return outputs, source_hash, contexto


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="verifica se os arquivos gerados correspondem a fonte e ao criterio")
    args = parser.parse_args()
    try:
        outputs, source_hash, contexto = expected_outputs()
        if args.check:
            mismatches = []
            for name, expected in outputs.items():
                path = Path(name)
                if not path.exists() or path.read_bytes() != expected:
                    mismatches.append(path.name)
            if mismatches:
                print("Subconjuntos divergentes: " + ", ".join(mismatches), file=sys.stderr)
                return 1
            print("Subconjuntos da Fase II reproduziveis verificados; "
                  f"dataset fonte SHA-256 {source_hash}.")
            return 0

        DESTINATION.mkdir(parents=True, exist_ok=True)
        latitude, longitude = contexto["centro"]
        print(f"Registros validos no dataset: {contexto['validos']}")
        print(f"Celula mais densa: centro em lat {latitude:.2f}, lon {longitude:.2f}, "
              f"com {contexto['densidade']} antenas")
        print()
        for name, content in outputs.items():
            path = Path(name)
            tamanho = int(path.stem.split("_n")[-1])
            path.write_bytes(content)
            print(f"{path.relative_to(ROOT)}: {len(content.splitlines()) - 1} vertices; "
                  f"raio {contexto['raios'][tamanho]:.2f} km; "
                  f"SHA-256 {hashlib.sha256(content).hexdigest().upper()}")
        print()
        print(f"Dataset fonte SHA-256: {source_hash}")
        print(f"Criterio: celula de {CELULA_GRAUS} grau mais densa; "
              "as N antenas mais proximas do centro dela")
    except (OSError, ValueError) as error:
        print(error, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())