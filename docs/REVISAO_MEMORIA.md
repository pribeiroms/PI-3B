# Revisao do gerenciamento de memoria (#28)

## Escopo

Foram revisados os pontos de alocacao e liberacao em `src/grafo.c` e os usos
associados a criacao, crescimento, conexao, deteccao de cruzamentos e destruicao
do grafo.

## Resultado

- Os blocos pertencentes ao grafo sao liberados em `grafo_destruir`; as listas
  encadeadas tambem sao percorridas e liberadas individualmente.
- As alocacoes de nos da lista tratam falha liberando qualquer no parcial antes
  de retornar.
- Os buffers temporarios da construcao de conexoes sao liberados em sucesso e
  falha; o buffer de cruzamentos e liberado internamente quando nao e solicitado
  pelo chamador, e sua propriedade e transferida ao chamador quando solicitado.
- O crescimento dos vetores, das cabecas de lista, da matriz, dos buffers
  temporarios e do resultado de cruzamentos agora verifica overflow antes de
  calcular o tamanho de alocacao.
- A desigualdade de Euler calcula `3(V-2)` com verificacao de limite para evitar
  overflow em `size_t`.
- Os testes existentes incluem crescimento das representacoes ate 1.000
  vertices e destruicao desses grafos.

## Validacao e limitacoes

`mingw32-make -B all test-integracao test-subconjuntos` passou, incluindo os
testes unitarios e os fluxos de integracao e estresse disponíveis.
`git diff --check` tambem passou.

O ambiente usa MinGW GCC 6.3.0, cuja instalacao nao inclui o runtime do
AddressSanitizer (`-lasan` ausente); Valgrind, Dr. Memory e Clang tambem nao
estao instalados. Portanto, esta revisao nao afirma ter medido leaks em tempo de
execucao com um detector dinâmico. A conclusao sobre liberacao decorre da
inspecao dos caminhos de propriedade e dos testes funcionais. Nao foi
identificado caminho atual de vazamento, ponteiro pendente ou acesso fora dos
limites; os riscos de overflow em tamanhos de alocacao foram corrigidos.
