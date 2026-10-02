# Guia de entrega

## Conteúdo esperado

- Código-fonte em C, com implementação autoral das estruturas e algoritmos de grafos.
- Arquivos de entrada usados nas execuções, em `data/`.
- Testes em `tests/` cobrindo os módulos implementados.
- Resultados reproduzíveis e sua interpretação.
- Documentação das decisões de projeto, complexidades e instruções de execução.

## Convenções sugeridas

- Um módulo possui um cabeçalho em `include/` e uma implementação correspondente em `src/`.
- Cada novo algoritmo deve ter testes de casos típicos, casos-limite e entradas inválidas.
- Não incluir binários nem arquivos temporários no repositório.

## Estado da entrega

**Ainda não liberada.** A análise da #17 foi consolidada nesta branch com a
contagem real da #16. O fluxo da #34 e a integração completa da #35 foram
validados nesta branch em 02/10/2026 com o dataset real completo em lista e
matriz. A auditoria da #36 está em [REVISAO_FASE1.md](REVISAO_FASE1.md);
benchmarks, artigo, revisão por outro integrante e integração da versão
aprovada em `main` ainda precisam ser concluídos.

## Documentos para conferência

- [Dataset e rastreabilidade](DATASET.md).
- [Modelagem](MODELAGEM_GRAFO.md).
- [Análise de planaridade](ANALISE_PLANARIDADE.md).
- [Estimativa de memória](MEDICAO_MEMORIA.md).
- [Protocolo experimental e subconjuntos de estresse](PROTOCOLO_EXPERIMENTAL.md).
- [Benchmark da lista de adjacência](BENCHMARK_LISTA.md).
- [Fluxo e formato dos resultados](FLUXO_APLICACAO.md).
- [Validação integrada e evidências](TESTE_INTEGRACAO_FASE1.md).

O artigo, a documentação completa dos algoritmos e os resultados finais de
benchmark devem ser disponibilizados pelos responsáveis para revisão.

## Sequência antes da liberação

1. Executar e revisar os benchmarks pendentes de matriz e os dados de gráficos.
2. Disponibilizar e revisar o artigo, documentação dos algoritmos e issues.
3. Obter revisão de outro integrante nos PRs, conforme `CONTRIBUTING.md`.
4. Integrar somente a versão validada em `main` e repetir a verificação nessa
   versão. Registrar o commit final e os comandos usados para reproduzi-la.

Não usar palavras de fechamento de issue em entregas parciais. Os arquivos
gerados em `results/` permanecem locais; combinar com a equipe como apresentar
as evidências finais sem violar a regra de não versionar saídas temporárias.
