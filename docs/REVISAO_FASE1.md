# Revisão do repositório — issue #36

**Revisão local atualizada em 02/10/2026. Situação: entrega final ainda não
liberada.** O fluxo funcional da aplicação e o teste integral da #35 foram
validados nesta branch; a branch principal, o estado remoto de PRs/issues, o
artigo e os resultados experimentais restantes não foram aprovados nesta
revisão.

## Resumo da auditoria local

| Verificação | Resultado |
| --- | --- |
| Compilação e testes | `mingw32-make test-integracao` executado nesta revisão; testes unitários e fluxo #34 passaram. O teste integral #35 foi executado nos modos lista e matriz. |
| Funcionamento | Dataset real completo processado; detalhes e limites em [TESTE_INTEGRACAO_FASE1.md](TESTE_INTEGRACAO_FASE1.md). |
| Organização | Código em `src/` e `include/`, testes em `tests/`, dados versionados em `data/` e documentação em `docs/`. |
| Dataset | Dataset OpenCelliD filtrado está versionado. A execução integral registrou hash e contagens no resumo de teste local. |
| Resultados experimentais | Há subconjuntos de estresse e CSV bruto do benchmark de lista. Não há benchmark equivalente da matriz nem conjunto completo de gráficos/análises finais. |
| Documentação | Guias de fluxo, modelagem, planaridade, memória, protocolo experimental e integração presentes. Este documento atualiza o estado obsoleto da revisão parcial. |
| Artigo | Não foi encontrado artigo/fonte de artigo entre os arquivos versionados (`git ls-files`). É necessário incluir o documento para revisar metodologia e conclusões. |
| Binários e saídas | `build/` e `results/` são ignorados; somente `results/.gitkeep` está versionado. |
| Branches e commits | Auditoria local abaixo; não representa confirmação atual do servidor GitHub. |

## Evidência da integração

Na branch local `feature/paula-dev`, `HEAD` é `012ec1f` (`test(integration):
validate full dataset on both structures #35`), com `origin/feature/paula-dev`
apontando para o mesmo commit no último estado local consultado. O comando
executado nesta revisão foi:

```powershell
mingw32-make test-integracao
```

Os cinco executáveis de teste e o roteiro de fluxo da #34 passaram. O roteiro
com `-ExigirCompleto` processa o dataset inteiro separadamente em lista e
matriz compacta. Os resultados registrados em
[TESTE_INTEGRACAO_FASE1.md](TESTE_INTEGRACAO_FASE1.md) mostram 61.933 vértices,
38.131 arestas e 10.211 cruzamentos em cada modo, sem divergência funcional.
Tempos e memória são estimativas/medições daquele ambiente e não comprovam
desempenho geral nem correção matemática completa. Falha de alocação não foi
injetada.

Os commits locais relevantes incluem `47fb17f` (#17), `79dc246` (#28),
`0abf6f4` (#34) e `012ec1f` (#35). Eles não incluem palavras de fechamento de
issue; commits por si só não fecham issues no GitHub.

## Branches e Pull Requests

No último retrato local, `main` e `feature/dev` apontam para `50396ee`, enquanto
`feature/paula-dev` está em `012ec1f`. A comparação local indica divergência:
`origin/feature/dev...HEAD` tem 14 commits exclusivos de cada lado; o diff de
`origin/main` até a branch contém 39 arquivos e inclui várias issues, não só
#34/#35. Portanto, não se deve tratar o conteúdo como já integrado nem abrir um
PR amplo sem revisar a base, o destino e o escopo.

Este ambiente não tem acesso autenticado ao GitHub para confirmar PRs abertos,
aprovações, checks, issues em aberto ou atualizar seus estados. As referências
`origin/*` são apenas o último retrato local consultado. A regra em
`CONTRIBUTING.md` exige revisão de outro integrante e integração por PR; não
fizemos merge direto na branch principal.

## Pendências para liberar a entrega

1. Abrir/revisar PRs com base e destino confirmados pela equipe; dividir o diff
   amplo por escopo quando necessário e obter aprovação de outro integrante.
2. Confirmar no GitHub o estado e a responsabilidade das issues relacionadas;
   fechar apenas as que foram integradas conforme o fluxo do projeto.
3. Executar e documentar o benchmark de matriz (#24), comparável ao da lista
   (#23), além dos gráficos/estudos de crescimento previstos em #25–#27.
4. Disponibilizar o artigo (#32/#33) e revisar se métodos, resultados e
   conclusões correspondem aos experimentos reproduzíveis.
5. Conferir a documentação de algoritmos (#31) e as limitações já descritas
   nos documentos técnicos.
6. Depois das aprovações, integrar a versão candidata em `main` pelo fluxo de
   PR e repetir compilação e teste integral a partir do commit integrado.

## Conclusão

A aplicação integrada atende ao fluxo operacional descrito nas issues #34 e
#35 na branch examinada e passou pela validação local completa. A revisão #36
continua aberta: não há evidência local suficiente para declarar a Fase I pronta
para entrega enquanto o PR não for revisado/integrado e os itens de artigo,
benchmarks e estado remoto não forem resolvidos.
