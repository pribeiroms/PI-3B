# Teste de integração da Fase I — #35 (parcial)

## Escopo executado

Foi exercitado o executável da #34 com o dataset real versionado, usando os
primeiros 1, 100 e 1.000 registros válidos. O recorte de 1.000 foi repetido para
verificar estabilidade de vértices, arestas, Euler e presença de cruzamentos.
Esta entrega não representa a execução completa de todos os requisitos da #35.

```powershell
mingw32-make test-integracao
```

O alvo compila o que estiver desatualizado, executa os testes unitários e o
roteiro do fluxo parcial da #34 e, então, o teste integrado com dataset real.
O roteiro retorna `0` quando as verificações disponíveis passam, mas registra
`parcial_validado`, nunca aprovação completa da Fase I. Qualquer divergência
interrompe o teste com erro.

Para exigir conclusão integral, executar após a compilação:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_integracao_fase1.ps1 -ExigirCompleto
```

No estado atual, esse modo retorna `2` após as verificações disponíveis,
indicando que a validação completa permanece bloqueada. Não é um erro da
aplicação nem aprovação das funcionalidades pendentes.

## Evidências reproduzíveis

Cada execução cria uma pasta própria `results/integracao-<identificador>/` com:

- `execucoes.csv`: resultados reais produzidos pela aplicação;
- arquivos `*.stdout.log` e `*.stderr.log`: saída e erros de cada cenário;
- `resumo.json`: estado final, argumentos, códigos de saída, pendências,
  ambiente, commit-base, alterações locais e hashes SHA-256 do dataset,
  executável e roteiro.

Os logs são capturados pelo teste; não constituem um subsistema próprio de logs
da aplicação. Resultados gerados permanecem fora do versionamento, conforme a
regra do repositório. O roteiro e este resumo são versionados.

## Resultado observado em 01/10/2026

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
esperados. Quantidade de cruzamentos e memória continuam sem valor medido.

Os valores acima são referências de regressão da versão atual, não uma prova
independente da correção dos algoritmos. O roteiro verifica o hash do dataset;
mudanças intencionais nos dados ou algoritmos exigem revisar essas referências,
sem apenas atualizar valores para esconder uma falha.

## Cobertura dos requisitos

| Requisito da #35 | Validação atual |
| --- | --- |
| Carregamento real | Validado nos recortes e conferido hash antes/depois |
| Construção do grafo | V/E conferidos entre terminal, CSV e referência |
| Lista de Adjacência | Exercitada na estrutura conjunta; testes unitários existentes passam |
| Matriz de Adjacência | Exercitada na estrutura conjunta; testes unitários existentes passam |
| Alternância entre estruturas | Bloqueada; modos exclusivos retornam erro explícito e não exportam resultados |
| Euler | Conferido com V/E e mensagem de limitação presente |
| Cruzamentos | Presença e conclusão coerentes; contagem pendente |
| Tempo | Quatro etapas registradas e valores válidos |
| Memória | Bloqueada; campo vazio com estado pendente |
| Logs | Captura de stdout/stderr por cenário e resumo da execução |
| Geração de resultados | CSV preserva execuções e concorda com terminal |

O roteiro da #34, executado como pré-requisito, cobre argumentos inválidos,
arquivos ausentes, dataset vazio, erro de gravação, preservação do dataset,
limite zero com entrada controlada e compatibilidade de argumentos posicionais.
Nenhum erro impeditivo novo da aplicação foi encontrado nos cenários disponíveis.

## O que falta para concluir

1. Concluir na #34 a seleção efetiva de lista e matriz, alinhada às APIs das
   #7/#8/#11; executar os mesmos recortes em cada modo e comparar os resultados
   funcionais, sem exigir tempos ou memória iguais.
2. Integrar a medição da #19; conferir unidade, escopo e coerência entre
   terminal/CSV e por estrutura.
3. Integrar a quantidade de cruzamentos da #16 e o relatório completo da #17;
   testar casos conhecidos com zero, um e múltiplos cruzamentos.
4. Executar novamente a aplicação completa, corrigir erros impeditivos e
   registrar novas evidências antes de fechar a #35.

O dataset inteiro contém 62.604 registros de dados; ele não foi processado
integralmente nesta validação. Foram usados recortes reais, não benchmarks ou
testes de estresse. A matriz simultânea e a construção por pares tornam a
execução integral uma validação de escala separada. Falhas de alocação também
não foram injetadas; permanece a limitação das APIs descrita na documentação
da #34. A suíte específica de planaridade da #21 não é substituída por estes
testes de integração.
