#!/usr/bin/env python3
"""Gera subconjuntos aninhados e reproduziveis do CSV OpenCelliD versionado."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "data" / "opencellid_brasil_filtrado.csv"
DESTINATION = ROOT / "data" / "subconjuntos"
SEED = b"PI-3B-issue-22-v1"
SIZES = (100, 500, 1000, 5000)
HEADER = b"radio,mcc,net,area,cell,lat,lon,range,samples"
SOURCE_SHA256 = "5EB50BD6954466F08ECA483B2D9DA18B50C297E54710532FB7D0646B2987F8DB"


def is_valid(raw: bytes) -> bool:
    body = raw.rstrip(b"\r\n")
    if len(body) > 510 or b'"' in body:
        return False
    try:
        fields = body.decode("ascii").split(",")
        if len(fields) != 9 or not fields[0] or len(fields[0]) > 15:
            return False
        for value in fields[1:5] + [fields[8]]:
            if not value.isdecimal() or int(value) > 0xFFFFFFFF:
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


def expected_outputs() -> tuple[dict[str, bytes], str]:
    source_bytes = SOURCE.read_bytes()
    source_hash = hashlib.sha256(source_bytes).hexdigest().upper()
    if source_hash != SOURCE_SHA256:
        raise ValueError("Hash do dataset fonte mudou; revise o protocolo antes de regenerar.")
    lines = source_bytes.splitlines(keepends=True)
    if not lines or lines[0].rstrip(b"\r\n") != HEADER:
        raise ValueError("Cabecalho inesperado no dataset fonte.")

    candidates: list[tuple[bytes, int, bytes]] = []
    for index, raw in enumerate(lines[1:], start=1):
        if not is_valid(raw):
            continue
        rank = hashlib.sha256(SEED + b"\0" + index.to_bytes(8, "big") + raw).digest()
        candidates.append((rank, index, raw))

    if len(candidates) < max(SIZES):
        raise ValueError(f"Dataset possui apenas {len(candidates)} registros validos.")

    ranked = sorted(candidates)
    outputs: dict[str, bytes] = {}
    for size in SIZES:
        selected = sorted(ranked[:size], key=lambda item: item[1])
        output = DESTINATION / f"opencellid_n{size}.csv"
        outputs[str(output)] = lines[0] + b"".join(item[2] for item in selected)
    return outputs, source_hash


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="verifica se os arquivos gerados correspondem a fonte e semente")
    args = parser.parse_args()
    try:
        outputs, source_hash = expected_outputs()
        mismatches = []
        if args.check:
            for name, expected in outputs.items():
                path = Path(name)
                if not path.exists() or path.read_bytes() != expected:
                    mismatches.append(path.name)
            if mismatches:
                print("Subconjuntos divergentes: " + ", ".join(mismatches), file=sys.stderr)
                return 1
            print(f"Subconjuntos reproduziveis verificados; dataset fonte SHA-256 {source_hash}.")
            return 0

        DESTINATION.mkdir(parents=True, exist_ok=True)
        for name, content in outputs.items():
            path = Path(name)
            path.write_bytes(content)
            print(f"{path.relative_to(ROOT)}: {len(content.splitlines()) - 1} vertices; "
                  f"SHA-256 {hashlib.sha256(content).hexdigest().upper()}")
        print(f"Dataset fonte SHA-256: {source_hash}")
        print(f"Semente: {SEED.decode('ascii')}")
    except (OSError, ValueError) as error:
        print(error, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
