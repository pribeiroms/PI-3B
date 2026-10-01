# Consolidação da análise — issue #17 (parcial)

O módulo `analise_planaridade` recebe resultados já calculados e apresenta
vértices, arestas, condição de Euler, informação sobre cruzamentos e conclusão
para as rotas de cabos. A chamada no `main` preserva as medições da #18.
Os algoritmos de Euler e de interseção não são reimplementados neste módulo.

## Dependências para concluir

- **#13:** a condição de Euler já está disponível em `origin/feature/dev` e é
  reutilizada pela aplicação.
- **#16:** falta disponibilizar e integrar a quantidade de cruzamentos, além
  do resultado booleano atual. Confirmar com o responsável o nome da função,
  o tipo retornado e se a unidade será pares de arestas que se cruzam ou pontos
  geométricos distintos. O relatório deve explicitar a unidade acordada.
- **#14 e #15:** são dependências da detecção da #16. Confirmar com seus
  responsáveis a conclusão da representação geométrica e da interseção;
  a existência de um detector inicial não comprova o fechamento dessas issues.

Enquanto a contagem não estiver disponível, o `main` passa `NULL` como último
argumento de `analise_planaridade_exibir`. A saída informa que a quantidade está
pendente e a consolidação é parcial, inclusive quando o detector retorna falso.
Não se transforma o retorno booleano em uma quantidade de cruzamentos.

Quando a #16 fornecer a contagem, obtê-la no trecho de análise geométrica do
`main` e passar o endereço dessa variável no último argumento. A contagem deve
corresponder ao mesmo grafo analisado por Euler. Se fornecida, ela determina
também a presença de cruzamentos no relatório. Não fechar a #17 antes de
integrar e validar essa execução com a contagem real.

## Interpretação e limites

A condição `E <= 3V - 6` é necessária para grafos simples planares com `V >= 3`.
Sua violação permite concluir que o grafo não é planar; seu atendimento não
comprova planaridade. Para menos de três vértices, essa condição não se aplica.

Cruzamentos no traçado geográfico exigem avaliação de isolamento ou alteração
das rotas dos cabos, mas não demonstram sozinhos que o grafo seja não planar.
Nenhum cruzamento encontrado pelo detector atual também não certifica
viabilidade física: o detector existente verifica cruzamentos próprios entre
arestas sem vértice comum; não cobre todos os contatos ou sobreposições
colineares. A evolução dessa geometria pertence às #14–#16.

Os testes deste módulo verificam apenas a consolidação e as mensagens, com
entradas controladas. Não substituem a suíte de planaridade da #21, nem a
integração geral da #34 e os testes completos da #35.

## Validação

Executar `mingw32-make` e `mingw32-make test`. O teste do relatório cobre
contagem pendente com e sem detecção, contagem fornecida e os três resultados
de Euler. `mingw32-make run` demonstra o estado parcial com o dataset real.
