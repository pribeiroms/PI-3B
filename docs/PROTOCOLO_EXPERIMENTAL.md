# Protocolo experimental — subconjuntos de estresse (#22)

## Origem e seleção

Os subconjuntos são derivados de `data/opencellid_brasil_filtrado.csv`, cuja
SHA-256 é
`5EB50BD6954466F08ECA483B2D9DA18B50C297E54710532FB7D0646B2987F8DB`.
O script `scripts/gerar_subconjuntos.py` usa a semente fixa
`PI-3B-issue-22-v1`. Para cada registro válido, calcula
`SHA-256(semente || 0x00 || ordinal_da_linha_em_8_bytes_big_endian || linha_original)`.
Ordena por esse digest e seleciona os primeiros N; os registros selecionados
são gravados na ordem em que aparecem na fonte. Assim, os quatro recortes são
determinísticos e aninhados. A seleção por hash distribui a amostra pela fonte
sem depender do algoritmo interno de `random.sample` de uma versão do Python.

Para regenerar os arquivos:

```powershell
python scripts/gerar_subconjuntos.py
python scripts/gerar_subconjuntos.py --check
```

O comando `--check` confirma os bytes gerados contra a fonte e a semente sem
alterar os arquivos. Python 3 é necessário para gerar ou conferir os recortes.

## Arquivos e contagens observadas

As contagens de vértices e arestas abaixo foram obtidas executando o programa
com cada arquivo, `--limite 0` e `--estrutura conjunta`. Um vértice corresponde
a cada registro carregado. As arestas seguem a regra de conexão descrita em
[MODELAGEM_GRAFO.md](MODELAGEM_GRAFO.md); são resultados desta versão do
algoritmo, não propriedades fixas do CSV.

| N | Arquivo | Vértices | Arestas | SHA-256 |
| ---: | --- | ---: | ---: | --- |
| 100 | `data/subconjuntos/opencellid_n100.csv` | 100 | 2 | `25774DBA4E864B0207E85712A8AFBDE29613D2405F0A4508B05CB889390517C7` |
| 500 | `data/subconjuntos/opencellid_n500.csv` | 500 | 48 | `4CACFCA245705FA633A75AB468B2C7E7623317D271FA9E75DD93ACA0EEB84D3E` |
| 1.000 | `data/subconjuntos/opencellid_n1000.csv` | 1.000 | 186 | `AF3657E93DEFB5F5B2E520F1FD53046D3672407BD0882F20ABDDFEE3D601FED8` |
| 5.000 | `data/subconjuntos/opencellid_n5000.csv` | 5.000 | 1.897 | `591E73D967705BA75593EB9706EDDF75CE459B3D3191F7E99423673852CF8F79` |

O conjunto de 5.000 registros foi incluído como recorte maior viável no
ambiente atual. Não foi usado o dataset completo: a implementação mantém uma
matriz de adjacência quadrada e compara pares de antenas ao construir o grafo.
Os testes de estresse medem o comportamento desta implementação conjunta; não
substituem execuções independentes de lista e matriz, que dependem da seleção
real de representação.

## Validação reproduzível

Na raiz do projeto, executar:

```powershell
mingw32-make test-subconjuntos
```

O teste verifica que os arquivos correspondem à fonte, reexecuta cada recorte,
confere V e E no terminal e no CSV, e exige zero registros inválidos. Os CSVs
de execução são temporários em `build/` e não são versionados.
