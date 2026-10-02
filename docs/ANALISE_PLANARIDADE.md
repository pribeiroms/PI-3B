# Análise consolidada da planaridade — issue #17

A aplicação reúne no relatório o número de vértices e arestas, o resultado da
condição de Euler, a quantidade de cruzamentos geométricos e a conclusão para
as rotas da rede. A contagem usa `grafo_detectar_cruzamentos` (#16), que compara
pares de arestas sem vértice comum usando a interseção de segmentos (#15).
O número é de pares de arestas que se cruzam; contatos colineares também são
contados conforme a regra da detecção geométrica.

A quantidade e a presença são obtidas na mesma execução do grafo analisado por
Euler. A conclusão indica quando cruzamentos no traçado pedem avaliação de
isolamento ou alteração das rotas. A condição `E <= 3V - 6` é necessária para
grafos simples planares com `V >= 3`: se violada, o grafo não é planar; se
atendida, o resultado é inconclusivo. Para menos de três vértices, a condição
não se aplica.

## Limites da análise

A fórmula de Euler, isoladamente, não constitui um teste completo de
planaridade. Cruzamentos no desenho geográfico indicam um problema de traçado,
mas não provam que o grafo seja não planar, pois outras posições dos vértices
podem permitir uma representação sem cruzamentos. Da mesma forma, ausência de
cruzamentos no desenho atual não certifica a viabilidade física da rede.

A contagem representa pares de arestas que se cruzam, não pontos geométricos
distintos. A detecção compara todos os pares de arestas e exclui arestas com
vértice comum. A integração e os casos de relatório são exercitados pelos
testes de `analise_planaridade`; os casos geométricos são cobertos pelos testes
de grafo.

## Validação

Na raiz do projeto, executar `mingw32-make all test`. A saída da aplicação com
o dataset real pode ser conferida com `mingw32-make run`.
