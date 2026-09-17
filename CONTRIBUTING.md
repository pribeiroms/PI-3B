# Guia de Contribuição

Este documento define o padrão de colaboração da equipe para manter o
repositório organizado, rastreável e pronto para entrega.

## Branches

- `main`: versão final, estável e entregue do projeto. Não recebe commits diretos.
- `develop`: branch de integração, compilação e testes das funcionalidades.
- `feature/paula-dev`
- `feature/miguel-dev`
- `feature/aluisio-dev`
- `feature/andre-dev`
- `feature/vitor-dev`

Cada integrante desenvolve em sua respectiva branch `feature/`. Caso uma tarefa
precise de uma branch temporária, ela deve partir da `develop` e seguir o formato
`feature/descricao-curta`, `fix/descricao-curta`, `docs/descricao-curta` ou
`test/descricao-curta`.

## Padrão de commits

Todo commit deve ter uma mensagem curta, no imperativo, e o número da issue
relacionada ao final.

```text
tipo(escopo): descrição curta #numero-da-issue
```

Tipos adotados:

- `feat:` nova funcionalidade.
- `fix:` correção de erro.
- `chore:` organização ou configuração.
- `docs:` documentação.
- `test:` testes.
- `refactor:` melhoria interna sem alterar a funcionalidade.
- `merge:` integração de funcionalidades.

Exemplo:

```sh
git commit -m "feat(graph): implementar lista de adjacencia #07"
```

Evite commits genéricos, como `update`, `teste` ou `alterações`. Commits de merge
devem ser criados pela plataforma no momento da integração; não os use para
alterações comuns de código.

## Fluxo de Pull Request

1. Crie ou selecione uma issue antes de iniciar a tarefa.
2. Desenvolva na branch individual ou temporária correspondente.
3. Atualize a branch local com a `develop` e resolva conflitos antes de abrir o PR.
4. Execute a compilação e os testes locais.
5. Abra um Pull Request da branch de trabalho para `develop`.
6. Descreva o objetivo, os arquivos alterados, como os testes foram executados e a issue relacionada.
7. Para fechar a issue após a integração, inclua `Closes #numero` na descrição do PR.
8. Após aprovação, faça o merge do PR. Apenas entregas estáveis devem seguir de `develop` para `main`.

## Revisão de código

- Todo Pull Request deve ser revisado por pelo menos outro integrante da equipe.
- O autor não aprova nem integra o próprio PR sem revisão.
- O revisor deve conferir compilação sem avisos, testes, legibilidade, tratamento de erros e aderência a este guia.
- Comentários de revisão devem ser resolvidos antes do merge.
- Alterações extensas devem ser divididas em PRs menores quando possível.

## Convenções para C

- Arquivos de código e cabeçalhos usam minúsculas e `snake_case`: `lista_adjacencia.c` e `lista_adjacencia.h`.
- Cada módulo público possui um cabeçalho em `include/` e a implementação correspondente em `src/`.
- Funções e variáveis usam `snake_case`: `grafo_adicionar_aresta`.
- Tipos e estruturas usam PascalCase: `Grafo`, `Vertice`.
- Constantes e macros usam maiúsculas com `_`: `NUMERO_MAXIMO_VERTICES`.
- Cabeçalhos usam guardas de inclusão: `#ifndef NOME_DO_ARQUIVO_H`.
- Uma função deve executar uma responsabilidade clara; funções públicas devem ser declaradas no respectivo `.h`.
- Código novo deve compilar com as flags definidas no `Makefile`, sem warnings.

## Integração na branch principal

- Não realizar commits ou pushes diretos em `main` ou `develop`.
- Só integrar um PR após revisão aprovada, compilação bem-sucedida e testes aprovados.
- `develop` deve permanecer integrada e funcional.
- A integração de `develop` em `main` ocorre somente em marcos de entrega ou quando a equipe validar a versão final.
- Não enviar executáveis, diretórios de build, resultados temporários ou arquivos de IDE ao repositório.

## Checklist antes de abrir um PR

- [ ] A issue está indicada no título, descrição ou commits.
- [ ] O projeto compila localmente.
- [ ] Os testes passam com `mingw32-make test`.
- [ ] Novos módulos possuem testes quando aplicável.
- [ ] Não há arquivos gerados ou alterações sem relação com a tarefa.
- [ ] A documentação foi atualizada quando necessário.
