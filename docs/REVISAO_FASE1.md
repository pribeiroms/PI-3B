# Revisão do repositório — issue #36

**Auditoria atualizada em 02/10/2026. A entrega final ainda não está liberada.**
A branch de trabalho passou na integração completa #35, mas essa alteração não
está em `main`; a validação reproduzível de `main`, a revisão por outro
integrante e os itens experimentais/editoriais pendentes impedem declarar uma
versão final estável.

## Código e validação

| Verificação | Resultado |
| --- | --- |
| Código e organização | Módulos C em `src/` e `include/`, testes em `tests/`, dataset em `data/` e guias em `docs/`. |
| Dataset | `data/opencellid_brasil_filtrado.csv` versionado; hash, registros válidos/inválidos e contagens da execução integral estão em [TESTE_INTEGRACAO_FASE1.md](TESTE_INTEGRACAO_FASE1.md). |
| Binários/resultados | `build/` e saídas de `results/` são ignorados; apenas `.gitkeep` está versionado em `results/`. |
| Branch `feature/paula-dev` | `c5d4368`; a integração completa #35 passou localmente em 02/10 com `mingw32-make test-integracao`, no dataset inteiro e nos modos lista e matriz. A revisão #35 é local até ser integrada. |
| Branch `main` | `128e69f` (merge do PR #56). A compilação C11 com `-Wall -Wextra -Wpedantic -Werror` passou e `grafo.exe --help` saiu com código 0. `mingw32-make all test` passou os dois primeiros testes, mas o Windows bloqueou `test_conexoes_geograficas.exe` por política de segurança (código 4551); portanto, a suíte completa da `main` não foi validada nesta máquina. |
| Integração na `main` | A árvore da `main` ainda tem o relatório parcial da #35 e documentação que declara a validação completa pendente. Não há evidência da execução integral nessa branch. |

O registro da execução completa de #35 mostra 61.933 vértices, 38.131 arestas
e 10.211 cruzamentos iguais para lista e matriz. A descrição dos resultados,
memória, tempos e limites está em [TESTE_INTEGRACAO_FASE1.md](TESTE_INTEGRACAO_FASE1.md).
Essa validação funcional não é benchmark controlado nem prova matemática
completa dos algoritmos.

## Commits, branches e Pull Requests

Consulta à API pública do GitHub e referências remotas feita em 02/10/2026:

- O PR #56 (`feature/paula-dev` → `main`) foi integrado em 02/10 pelo autor,
  sem registros de revisão formal. O PR tem 39 arquivos e seu `head` é
  `0abf6f4` (#34); ele não inclui o commit posterior `012ec1f` que valida a #35
  nem este relatório atualizado.
- Não havia PR aberto na consulta. A branch atual `feature/paula-dev` está em
  `c5d4368`; `main` está em `128e69f`. O PR #56 integrou o fluxo #34, mas o
  commit da validação completa #35 não está em `main`.
- A comparação local do estado atualizado mostra a branch de trabalho com dois
  commits após a base comum e a `main` com um commit após essa base. É necessário
  abrir/revisar um PR para integrar #35 e as atualizações deste documento.
- `CONTRIBUTING.md` exige revisão de outro integrante. A integração do PR #56
  sem registro de revisão não atende esse controle e deve ser revisada pela
  equipe.

## Issues e materiais de entrega

Estados consultados no GitHub em 02/10/2026:

| Issue | Estado remoto | Evidência local / observação |
| --- | --- | --- |
| #24 benchmark da matriz | Fechada | Não há CSV/script de benchmark da matriz no inventário versionado desta branch. Confirmar onde ficou a evidência do fechamento. |
| #25 comparação lista/matriz | Fechada | Não foi encontrado relatório comparativo próprio nos arquivos versionados. |
| #26 crescimento assintótico | Aberta | Entrega de análise experimental pendente. |
| #27 dados para gráficos | Aberta | Dados/gráficos finais pendentes. |
| #29 documentação do README | Aberta | README existe; confirmar se a issue foi revisada/encerrada pela equipe. |
| #31 documentação dos algoritmos | Aberta | Documentação completa ainda não identificada. |
| #32 metodologia do artigo | Aberta | Nenhum arquivo de artigo/fonte está versionado (`git ls-files`). |
| #33 análise dos resultados do artigo | Aberta | Depende do artigo e dos resultados revisados. |
| #34 fluxo principal | Fechada | Integrado pelo PR #56. |
| #35 teste integral da Fase I | Aberta | Passou na branch `feature/paula-dev`; falta integrar e validar a versão candidata em `main`. |
| #36 revisão final | Aberta | Este documento registra a auditoria atualizada; permanecem os bloqueios listados abaixo. |

As issues #16/#17/#19/#22/#23/#28 e demais componentes consultados aparecem
fechados. O CSV de benchmark de lista está versionado. O estado fechado de #24 e
#25 foi confirmado, mas a evidência correspondente não está no inventário do
repositório revisado; não assumir que o item experimental está reproduzível sem
localizar essa entrega.

## Pendências para liberar a Fase I

1. Fazer revisão de outro integrante e abrir PR para os commits posteriores ao
   merge #56, incluindo o teste completo da #35; confirmar o destino e evitar
   integrar diretamente em `main`.
2. Repetir compilação e `mingw32-make test-integracao` numa cópia limpa da
   versão candidata já baseada na `main`; resolver ou documentar o bloqueio
   local do Windows App Control durante `test_conexoes_geograficas.exe`.
3. Localizar/versionar evidências reproduzíveis de #24/#25 e concluir #26/#27.
4. Receber e revisar os materiais #31–#33 (algoritmos e artigo SBC), verificando
   que metodologia e conclusões correspondam ao código e aos dados.
5. Confirmar a resolução de #29 e os checks do PR no GitHub; fechar #35/#36
   somente depois da integração aprovada e da validação final em `main`.

## Conclusão

O fluxo #34 está em `main`; a validação completa #35 passou na branch de
trabalho, mas ainda não foi integrada. `main` compila e a ajuda executa nesta
máquina, porém a suíte foi interrompida pelo bloqueio de segurança e a árvore
não contém a validação integral. Assim, não é possível certificar `main` como a
versão estável e reproduzível exigida pela #36. A issue deve permanecer aberta
até a revisão de PR, a integração e as pendências experimentais/editoriais serem
resolvidas.
