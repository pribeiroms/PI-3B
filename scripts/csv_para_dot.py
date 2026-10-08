#!/usr/bin/env python3
"""Converte o dataset OpenCelliD para Graphviz DOT usando as regras do projeto."""

import argparse
import csv
import math
from pathlib import Path


def distancia_metros(a, b):
    rad = math.pi / 180
    dlat = (b["lat"] - a["lat"]) * rad
    dlon = (b["lon"] - a["lon"]) * rad
    x = math.sin(dlat / 2) ** 2 + math.cos(a["lat"] * rad) * math.cos(b["lat"] * rad) * math.sin(dlon / 2) ** 2
    return 6371000 * 2 * math.atan2(math.sqrt(x), math.sqrt(1 - x))


def quote(value):
    return '"' + str(value).replace("\\", "\\\\").replace('"', '\\"') + '"'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", type=Path)
    parser.add_argument("dot", type=Path)
    parser.add_argument("--limit", type=int, default=1000, help="registros iniciais; 0 carrega todos (padrão: 1000)")
    args = parser.parse_args()

    with args.csv.open(encoding="utf-8-sig", newline="") as source:
        reader = csv.DictReader(source)
        required = {"radio", "mcc", "net", "area", "cell", "lat", "lon", "range", "samples"}
        if not required.issubset(reader.fieldnames or []):
            parser.error("CSV não possui o cabeçalho OpenCelliD esperado")
        vertices = []
        for row in reader:
            try:
                v = {"radio": row["radio"], "mcc": int(row["mcc"]), "net": int(row["net"]),
                     "area": int(row["area"]), "cell": int(row["cell"]), "lat": float(row["lat"]),
                     "lon": float(row["lon"]), "range": float(row["range"])}
                if not (-90 <= v["lat"] <= 90 and -180 <= v["lon"] <= 180 and v["range"] >= 0):
                    continue
                vertices.append(v)
            except (TypeError, ValueError):
                continue
            if args.limit > 0 and len(vertices) >= args.limit:
                break

    nearest = [None] * len(vertices)
    shortest = [math.inf] * len(vertices)
    for i, a in enumerate(vertices):
        for j in range(i + 1, len(vertices)):
            b = vertices[j]
            if (a["mcc"], a["net"], a["area"]) != (b["mcc"], b["net"], b["area"]):
                continue
            distance = distancia_metros(a, b)
            if distance > a["range"] + b["range"]:
                continue
            if distance < shortest[i]:
                shortest[i], nearest[i] = distance, j
            if distance < shortest[j]:
                shortest[j], nearest[j] = distance, i

    edges = set()
    for i, j in enumerate(nearest):
        if j is not None and (nearest[j] != i or i < j):
            edges.add((min(i, j), max(i, j)))

    args.dot.parent.mkdir(parents=True, exist_ok=True)
    with args.dot.open("w", encoding="utf-8", newline="\n") as output:
        output.write("// Gerado de {}: {} vértices, {} arestas.\n".format(args.csv.name, len(vertices), len(edges)))
        output.write('graph Antenas {\n  graph [layout=neato, overlap=false, splines=true];\n  node [shape=circle, style=filled, fillcolor="#dbeafe", color="#2563eb", fontsize=8];\n  edge [color="#64748b"];\n')
        for i, v in enumerate(vertices):
            label = f'{v["cell"]} ({v["radio"]})'
            position = f'{v["lon"]:.6f},{v["lat"]:.6f}!'
            tooltip = f'MCC {v["mcc"]}, operadora {v["net"]}, área {v["area"]}'
            output.write(f'  n{i} [label={quote(label)}, pos={quote(position)}, tooltip={quote(tooltip)}];\n')
        for i, j in sorted(edges):
            output.write(f"  n{i} -- n{j};\n")
        output.write("}\n")
    print(f"DOT criado: {args.dot} ({len(vertices)} vértices, {len(edges)} arestas)")


if __name__ == "__main__":
    main()
