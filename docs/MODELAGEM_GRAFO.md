# Modelagem da Fase I — Telecomunicações

## Grafo geográfico

O problema é representado por um grafo simples, não direcionado e sem peso
`G = (V, E)`. Cada linha válida do dataset OpenCelliD representa uma antena e,
portanto, um vértice de `V`. A posição do vértice é dada por `lat` e `lon`.
Cada elemento de `E` representa uma rota possível de cabo subterrâneo entre duas
antenas e é desenhado como o segmento de reta entre suas coordenadas.

## Regra de conexão

Duas antenas são elegíveis para conexão quando:

1. possuem o mesmo país (`mcc`), operadora (`net`) e área (`area`); e
2. a distância geográfica entre elas é menor ou igual à soma de seus alcances
   estimados (`range`).

Quando há mais de uma antena elegível, a implementação cria a aresta apenas com
a elegível mais próxima. Essa escolha evita uma malha completa de cabos, que não
representaria uma planta econômica, e mantém as arestas sem peso como exigido na
Fase I.

## Verificação da planta

O programa compara pares de arestas que não compartilham vértices. Caso dois
segmentos se interceptem, a resposta é que a planta exige isolamento ou uma nova
rota para o cabo. Caso contrário, não foram identificados cruzamentos na amostra
analisada.

## Execução

Por padrão são processadas as primeiras 1.000 antenas, um tamanho adequado para
a validação inicial por comparação de segmentos:

```sh
mingw32-make run
```

Também é possível informar arquivo e limite de registros:

```sh
build/grafo.exe data/opencellid_brasil_filtrado.csv 1000
```

Use `0` como limite para carregar todos os registros. A verificação de
cruzamentos compara pares de arestas e, por isso, é recomendável analisar o
dataset completo por recortes geográficos nas próximas otimizações.


## Validação de planaridade (Euler)

Com base na fórmula de Euler, todo grafo simples planar com V >= 3 satisfaz:

`E <= 3V - 6`

onde V é o número de vértices e E é o número de arestas.
Essa desigualdade é usada para validar a planaridade do grafo, mas é uma
condição **necessária, não suficiente**:

**Violada** (E > 3V - 6): o grafo é certamente **não planar**.
**Aceita** (E <= 3V - 6): o resultado é **inconclusivo**. A análise
  baseada apenas em Euler não garante que o grafo seja planar
  (contraexemplo: K3,3, com V = 6 e E = 9, que satisfaz a condição
  mas não é planar).
**V < 3**: a condição não se aplica.
