# Revisão do repositório — #36 (parcial)

**Situação: não liberado para entrega final.** Revisão realizada em 01/10/2026
na `feature/paula-dev`, com base no commit `f6fa594`. Não houve merge em `main`,
fechamento de issues ou aprovação de PRs nesta revisão.

## Verificações realizadas

| Item da #36 | Resultado e limite |
| --- | --- |
| Compilação | Cópia local independente do commit `f6fa594`, sem binários anteriores, compilada com C11 e `-Wall -Wextra -Wpedantic -Werror` |
| Funcionamento | Testes unitários, fluxo parcial e integração disponível passaram nessa cópia |
| Organização | Módulos em `src/` e `include/`, testes em `tests/`, documentos em `docs/`, dataset em `data/` |
| Commits | `bcd6128`, `060c64f` e `f6fa594` referenciam #17, #34 e #35 sem palavras de fechamento |
| Pull Requests | Aprovações e destino efetivo ainda precisam ser confirmados no GitHub; histórico de merges não comprova revisão dos commits atuais |
| Branches | Referências locais mostram `feature/dev` como integração; discrepância com `develop` registrada em `CONTRIBUTING.md` |
| README | Corrigidos texto de base inicial, próximos passos já executados e pré-requisitos; estado parcial explícito |
| Dataset | Arquivo versionado: 62.604 registros; contagens por tecnologia e hash conferidos |
| Resultados experimentais | Registros dos recortes reais reproduzidos pela #35; não são benchmarks finais de lista versus matriz |
| Documentação | Índice de entrega e dependências consolidados; documentação completa dos algoritmos ainda requer revisão da entrega do responsável |
| Artigo | Nenhum arquivo de artigo identificado no inventário versionado da branch; solicitar fonte/documento e conteúdo para revisão |
| Issues pendentes | Dependências conhecidas registradas abaixo; lista atual completa e responsáveis precisam de confirmação no GitHub |
| Versão principal | `origin/main` local aponta para `50396ee`; não contém os três commits parciais acima. A estabilidade da versão final de `main` ainda não foi validada |

As referências remotas acima são o retrato local consultado, não uma auditoria
atualizada dos estados de PRs e issues no servidor. A consulta web disponível
não permitiu confirmar os PRs atuais e mostrou dados de issues em cache;
por isso não é usada para declarar aprovações ou fechamentos.

## Reprodutibilidade conferida

Foi criado um clone local independente, sem hardlinks, da `feature/paula-dev`
em uma pasta nova de `build/`. Nesse clone foram executados:

```powershell
mingw32-make all test-integracao
```

Resultado: compilação sem avisos com MinGW GCC 6.3.0, cinco executáveis de testes
unitários aprovados, roteiro da #34 aprovado e validação parcial da #35 aprovada.
A validação de integração usa 1, 100 e 1.000 registros reais, com repetição de
1.000. Não processa todo o dataset, não certifica alternância entre estruturas
e não mede memória. O procedimento valida os arquivos versionados da branch,
mas não substitui um clone do remoto e teste do commit final de `main`.

Os logs e CSVs foram gerados no diretório `results/integracao-<id>/` dessa cópia,
com parâmetros, hashes e metadados de reprodução. Apenas `results/.gitkeep`
está versionado em `results/`; não foram encontrados executáveis, objetos ou
logs versionados. As saídas temporárias permanecem ignoradas pelo Git.

## Correções documentais desta revisão

- README descreve o estado implementado e o ambiente realmente validado.
- Guia de contribuição registra as diferenças entre nomes planejados e reais
  das branches, sem renomear branches dos colegas.
- Documento do dataset deixa claro que pesos e filtros por tecnologia/região
  não fazem parte da interface atual; a regra descrita corresponde ao código.
- Guia de entrega passa a apontar os documentos existentes e os critérios de
  liberação, sem apresentar uma entrega parcial como concluída.

## Pendências para concluir a #36

1. **#16/#17:** integrar e validar quantidade de cruzamentos e concluir análise.
2. **#19 e #34:** medir memória e selecionar efetivamente lista ou matriz com
   as APIs das estruturas (#7/#8/#11); concluir o fluxo principal.
3. **#35:** executar a validação completa após as integrações, corrigindo erros
   impeditivos e registrando evidências finais.
4. **Resultados:** disponibilizar benchmarks, comparação entre estruturas,
   crescimento experimental e dados finais para gráficos (#22–#27), conforme
   o escopo de cada responsável; as medições atuais não substituem esses itens.
5. **Memória e documentação:** conferir a entrega da #28 e a documentação dos
   algoritmos (#31). Permanecem limitações conhecidas: retorno ambíguo de falhas
   de alocação e geometria que não cobre todos os contatos/sobreposições.
6. **Artigo:** disponibilizar e revisar metodologia e análise dos resultados
   (#32/#33), alinhadas ao que foi efetivamente implementado e medido.
7. **GitHub:** confirmar issues/PRs atuais, resolver pendências de revisão com
   outro integrante e confirmar a branch de destino da integração.
8. **Entrega:** integrar a versão aprovada em `main`, executar novamente em
   cópia limpa e registrar o commit final reproduzível antes de fechar a #36.

Esta lista identifica entregas a conferir, não afirma que todas as issues
citadas continuam abertas no GitHub. Nenhum artigo, resultado experimental ou
aprovação de colega foi substituído por uma declaração de conclusão nesta revisão.
