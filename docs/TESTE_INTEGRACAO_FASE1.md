# Teste de integração da Fase I — #35

## Escopo executado

O executável foi validado com recortes e com todos os registros do dataset real
versionado. A execução integral foi repetida nos modos exclusivos de lista e
matriz; também houve comparação funcional dos dois modos com o conjunto em
N=1.000.

```powershell
mingw32-make test-integracao
```

O alvo compila o que estiver desatualizado, executa os testes unitários, o
roteiro da #34 e o teste da #35 com `-ExigirCompleto`. Esse último executa o
dataset inteiro sem limite, em lista e em matriz, e retorna `0` somente quando
as verificações passam. Qualquer divergência interrompe o teste com erro.

Para executar somente os oito cenários de recorte:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_integracao_fase1.ps1
```

O modo sem `-ExigirCompleto` registra `parcial_validado`; o alvo
`test-integracao` usa o modo completo e registra `integracao_validada`.

## Evidências reproduzíveis

Cada execução cria uma pasta própria `results/integracao-<identificador>/` com:

- `execucoes.csv`: resultados reais produzidos pela aplicação;
- arquivos `*.stdout.log` e `*.stderr.log`: saída e erros de cada cenário;
- `resumo.json`: estado final, argumentos, códigos de saída, pendências,
  ambiente, commit-base, alterações locais e hashes SHA-256 do dataset,
  executável e roteiro.

Os logs são capturados pelo teste; não constituem um subsistema próprio de logs
da aplicação. Resultados gerados permanecem fora do versionamento, conforme a
regra do repositório. O roteiro e esta documentação são versionados.

## Resultado histórico dos recortes em 01/10/2026

Ambiente: Windows, MinGW GCC 6.3.0. Base da aplicação: commit `060c64f`.
Dataset: `data/opencellid_brasil_filtrado.csv`.
SHA-256: `5EB50BD6954466F08ECA483B2D9DA18B50C297E54710532FB7D0646B2987F8DB`.

| Limite | Vértices | Arestas | Euler | Presença de cruzamentos |
| --- | --- | --- | --- | --- |
| 1 | 1 | 0 | Condição não aplicável | Não detectada |
| 100 | 100 | 33 | Condição atendida; inconclusivo | Não detectada |
| 1.000 | 1.000 | 637 | Condição atendida; inconclusivo | Detectada |
| 1.000 (repetição) | 1.000 | 637 | Condição atendida; inconclusivo | Detectada |

Nenhum registro inválido foi reportado nesses recortes. Os tempos por etapa
foram exportados e validados como números finitos não negativos. Zero é
permitido pela resolução do relógio; os tempos não são fixados como valores
esperados. Os resultados desta execução documentada são da base anterior à
integração da contagem. O roteiro agora verifica a quantidade no terminal e no
CSV; a medição de memória foi integrada depois dessa execução documentada.

Os valores acima são referências de regressão da versão atual, não uma prova
independente da correção dos algoritmos. O roteiro verifica o hash do dataset;
mudanças intencionais nos dados ou algoritmos exigem revisar essas referências,
sem apenas atualizar valores para esconder uma falha.

## Execução integral observada em 02/10/2026

Comando: `mingw32-make test-integracao`. Ambiente: Windows, MinGW GCC 6.3.0,
processo de 32 bits. Dataset SHA-256:
`5EB50BD6954466F08ECA483B2D9DA18B50C297E54710532FB7D0646B2987F8DB`.
O arquivo contém 62.604 registros; 61.933 foram carregados e 671 inválidos
foram ignorados. As duas representações produziram os mesmos resultados:

| Estrutura | Vértices | Arestas | Euler | Cruzamentos | Memória estimada da estrutura |
| --- | ---: | ---: | --- | ---: | ---: |
| Lista | 61.933 | 38.131 | Inconclusivo | 10.211 | 4.542.304 bytes |
| Matriz compacta | 61.933 | 38.131 | Inconclusivo | 10.211 | 540.540.976 bytes |

Os tempos totais observados foram 55.576 ms para lista e 55.097 ms para matriz.
São medidas de CPU deste ambiente, não benchmarks comparativos controlados.
O CSV, stdout, stderr e `resumo.json` dos dez cenários estão em
`results/integracao-fdb75e06ea7b47fbbd6212ecd53862c3/` nesta cópia local.
O resumo registra `integracao_validada`, dez casos, dez resultados e nenhuma
pendência.

## Cobertura dos requisitos

| Requisito da #35 | Validação atual |
| --- | --- |
| Carregamento real | Dataset completo carregado; hash conferido antes/depois; inválidos contabilizados |
| Construção do grafo | V/E coincidem entre lista e matriz no dataset completo |
| Lista de Adjacência | Modo exclusivo executado com todos os registros |
| Matriz de Adjacência | Modo exclusivo executado com todos os registros em representação compacta |
| Alternância entre estruturas | Lista e matriz executadas em sequência e comparadas com resultados idênticos |
| Euler | Resultado conferido com V/E; condição atendida e planaridade inconclusiva |
| Cruzamentos | Contagem 10.211 conferida no terminal e no CSV para ambas as estruturas |
| Tempo | Cinco etapas registradas com valores finitos não negativos |
| Memória | Estimativas da lista e da matriz registradas; matriz compacta ocupa cerca de 516 MiB estimados |
| Logs | Captura de stdout/stderr por cenário e resumo da execução |
| Geração de resultados | Dez linhas CSV preservadas e conferidas contra terminal e resumo |

O roteiro da #34, executado como pré-requisito, cobre argumentos inválidos,
arquivos ausentes, dataset vazio, erro de gravação, preservação do dataset,
limite zero com entrada controlada e compatibilidade de argumentos posicionais.
Nenhum erro impeditivo novo da aplicação foi encontrado nos cenários disponíveis.

## Limites e dependências relacionadas

Esta integração valida o fluxo e a concordância entre estruturas; não prova a
correção matemática completa dos algoritmos. A regra de Euler retorna
inconclusivo quando a condição necessária é atendida, como esperado. A suíte
específica de planaridade da #21 não é substituída por este teste. Falhas de
alocação também não foram injetadas.

O benchmark da matriz (#24) é uma issue separada: medir desempenho não é uma
dependência funcional para a validação da #35. A metodologia e os limites da
estimativa de memória estão em [MEDICAO_MEMORIA.md](MEDICAO_MEMORIA.md). Os
subconjuntos específicos de estresse continuam disponíveis em
[PROTOCOLO_EXPERIMENTAL.md](PROTOCOLO_EXPERIMENTAL.md) e são executados por
`mingw32-make test-subconjuntos`.
