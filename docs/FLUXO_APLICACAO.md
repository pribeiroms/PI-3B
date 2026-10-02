# Fluxo da aplicação — #34 (parcial)

O executável integra os componentes atualmente disponíveis, reutilizando
dataset, construção do grafo, Euler, contagem de cruzamentos,
medições por etapa e relatório da #17. Não reimplementa os algoritmos dos colegas.

## Executar

Na raiz do repositório:

```powershell
mingw32-make
./build/grafo.exe --help
./build/grafo.exe --dataset data/opencellid_brasil_filtrado.csv --limite 1000 --estrutura lista --saida results/execucoes.csv
```

Sem argumentos, usa o dataset versionado, limite de 1.000 vértices e salva em
`results/execucoes.csv`. A forma anterior `grafo.exe arquivo.csv 1000` continua
válida. Não misturar argumentos posicionais com opções nomeadas.

O limite é o máximo de registros válidos carregados (vértices), não o número de
linhas examinadas ou uma quantidade exata garantida. `0` lê todos os registros
válidos; use inicialmente recortes pequenos, pois a estrutura atual mantém uma
matriz e a construção compara pares de antenas. Valores negativos, fracionários,
texto, excesso numérico e opções repetidas são rejeitados.

## Resultados e medições

A saída apresenta parâmetros, registros inválidos ignorados, tempos por etapa,
vértices, arestas, Euler, quantidade de cruzamentos e conclusão da #17.

O CSV acrescenta uma linha por execução, com data UTC, caminho do dataset,
limite solicitado, modo efetivamente usado, resultados e tempos. O cabeçalho é
gravado uma vez. Um arquivo existente com outro cabeçalho é recusado, sem
sobrescrever seu conteúdo. Caminhos com vírgulas e aspas são escapados no CSV.
O diretório de saída precisa existir; `results/` já faz parte do repositório.
Não executar gravações concorrentes no mesmo arquivo. Os CSVs gerados não são
versionados, conforme `.gitignore`.

As medidas reutilizam `clock()` da #18 e são registradas em milissegundos sob
colunas `*_cpu_ms`. A resolução e a semântica dependem da biblioteca C usada;
não devem ser tratadas como benchmarks independentes de lista versus matriz.
Zero pode representar uma etapa abaixo da resolução do relógio; `-1` indica
relógio indisponível. Não incluem apresentação ou escrita do CSV.

O CSV registra a contagem de cruzamentos calculada pela #16 e a estimativa de
memória das estruturas com o estado `estimada_modelo_alocacoes`. A execução
integrada segue marcada como `parcial` porque a validação completa da #35 ainda
está pendente. O CSV é um registro básico de execução da #34, não
substitui a organização dos benchmarks e dados para gráficos das outras issues.

Códigos de saída: `0` para execução bem-sucedida ou ajuda, `1` para erro de
execução/gravação e `2` para argumentos inválidos.

## Requisitos e dependências

| Item da #34 | Situação |
| --- | --- |
| Carregar dataset | Integrado (#9) |
| Selecionar tamanho | Integrado como limite de vértices |
| Selecionar lista ou matriz | Modos exclusivos e conjunto implementados; testados em N=100 |
| Construir grafo | Integrado (#10) |
| Euler | Integrado (#13) |
| Cruzamentos | Contagem integrada (#16) |
| Apresentar resultados | Relatório completo da #17 integrado |
| Tempo | Medições disponíveis da #18 integradas |
| Memória | Estimativas comparáveis da lista e da matriz integradas (#19) |
| Salvar resultados | CSV básico implementado, com campos pendentes explícitos |

O padrão continua sendo `--estrutura conjunta` para compatibilidade. Para
medições independentes, `--estrutura lista` aloca e consulta a lista encadeada;
`--estrutura matriz` aloca e consulta a matriz. O campo `estrutura` no CSV
identifica o modo realmente utilizado. Cada modo deve ser comparado com os
mesmos arquivos de entrada e verificar se os resultados funcionais coincidem.

Para memória, o terminal e o CSV mostram bytes comuns, total estimado para a
lista e total estimado para a matriz. A estimativa soma o tamanho dos blocos
solicitados ao alocador: estrutura do grafo, capacidades reservadas para
vértices e arestas, vetor de cabeças e nós da lista, ou células da matriz.
Overhead interno do alocador, fragmentação e buffers temporários dos algoritmos
ficam de fora. Nos modos exclusivos, o total da representação selecionada
corresponde aos blocos solicitados pelo grafo e o total da estrutura ausente é
zero. No modo conjunto, ambos os totais são apresentados.

Para cruzamentos, o relatório e o CSV contam pares de arestas que se cruzam na mesma
execução analisada por Euler.

A API de construção existente retorna zero tanto para ausência de arestas como
para algumas falhas de alocação; a leitura também não distingue todos os erros
de alocação dos registros inválidos. Não é possível garantir a detecção desses
erros pela integração atual. Alinhar um retorno de erro explícito nas APIs antes
de considerar o fluxo validado para falhas de recursos.

## Validação desta entrega

```powershell
mingw32-make all test
powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_fluxo_parcial.ps1
```

O roteiro verifica o fluxo com recortes do dataset real, preservação de
execuções no CSV, campos não medidos, limites, ajuda, estruturas exclusivas, falhas
de leitura/gravação e compatibilidade posicional. Usa arquivos isolados em
`build/`. Não encerra a #35: ainda falta a análise completa para o teste de
integração de toda a Fase I.
