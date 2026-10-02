 ## Algoritmos e Estruturas Implementados
 
Esta seção documenta os principais algoritmos e estruturas de dados implementados na Fase I, todos escritos em linguagem C no módulo do grafo. São descritas a lista de adjacência, a matriz de adjacência, a construção das conexões, a validação pela fórmula de Euler, o algoritmo de interseção de segmentos e a análise de cruzamentos, seguidos das complexidades de tempo e de espaço e das limitações das abordagens utilizadas.
 
## Convenções
 
Nesta seção, V denota o número de vértices (antenas), E o número de arestas, deg(v) o grau do vértice v e Δ o grau máximo do grafo. A ordem alocada da matriz e do vetor de listas é representada por ordem e vale 128·2^k, com k inteiro, sendo o menor valor dessa forma maior ou igual a V; assim, V ≤ ordem < 2V para V > 128. Os tamanhos em bytes assumem uma plataforma de 64 bits, na qual um nó de adjacência e uma aresta ocupam 16 bytes cada e um vértice ocupa 48 bytes. O grafo é simples, não direcionado e sem peso, e laços e arestas paralelas são rejeitados na inserção de arestas.
 
## Estrutura central
 
O tipo Grafo é opaco, definido apenas no arquivo de implementação, e mantém três representações sincronizadas: o vetor de vértices, que guarda para cada antena o identificador, o mcc, o net, a area, a cell, as coordenadas e o alcance em metros; o vetor de arestas, que guarda os pares (origem, destino) na ordem de inserção; a lista de adjacência; e a matriz de adjacência.
 
Os invariantes são os seguintes. O identificador de um vértice é o seu índice no vetor de vértices. Cada aresta {u, v} aparece uma vez no vetor de arestas, duas vezes na lista de adjacência, uma em cada extremo, e em duas células simétricas da matriz. Os vetores de vértices e de arestas começam com 128 posições e dobram de capacidade quando cheios. A ordem da matriz e do vetor de listas também começa em 128 e dobra quando V atinge ordem.
 
## Lista de adjacência
 
A lista de adjacência é um vetor de ponteiros para listas encadeadas de nós, cada nó contendo o índice do vizinho e o ponteiro para o próximo nó. A aresta {u, v} gera um nó na lista de u e outro na lista de v. A inserção é feita no início da lista, de modo que a vizinhança é percorrida do vizinho mais recente para o mais antigo, comportamento verificado nos testes automatizados.
 
A consulta de adjacência percorre a lista da origem até encontrar o destino, com custo proporcional ao grau da origem. A função de vizinhos devolve a cabeça da lista, ou ponteiro nulo quando a lista está vazia ou o índice é inválido, e percorrer toda a vizinhança custa O(deg(v)). A liberação de memória percorre as ordem listas e libera cada nó.
 
## Matriz de adjacência
 
A matriz de adjacência é um vetor linear de ordem × ordem bytes, em que a célula (u, v) fica na posição u·ordem + v. Ela é simétrica, por tratar-se de grafo não direcionado, e a diagonal é sempre nula. A inserção de uma aresta grava o valor 1 nas células (u, v) e (v, u).
 
Quando V atinge ordem, a matriz é expandida: aloca-se com calloc uma nova matriz de nova_ordem² bytes, copia-se linha a linha a matriz antiga, pois as linhas têm tamanhos diferentes e um realloc simples não bastaria, e libera-se a antiga. Durante a cópia coexistem as duas matrizes, o que produz um pico de memória de ordem_antiga² + nova_ordem², isto é, 1,25 vez o tamanho final. A construção das conexões zera a matriz inteira antes de recriar as arestas.
 
A consulta pela matriz acessa diretamente a célula (u, v) em tempo O(1). No código original a matriz é apenas escrita e redimensionada, e nenhuma operação a lê; a consulta pela matriz só existe quando o acessor proposto na documentação dos experimentos é aplicado.
 
## Vetor de arestas
 
O vetor de arestas guarda os pares (origem, destino) na ordem de inserção e é a base da análise geométrica. A função de obtenção de segmento devolve as coordenadas dos dois extremos de uma aresta, e a função de compartilhamento de vértice compara os extremos de duas arestas, ambas em tempo O(1).
 
## Construção das conexões
 
Duas antenas são elegíveis para conexão quando possuem o mesmo mcc, net e area e a distância entre elas é menor ou igual à soma de seus alcances. A distância é a de Haversine, em metros, com raio terrestre de 6.371.000 m.
 
O algoritmo procede em quatro passos. No primeiro, remove todas as arestas existentes, limpando listas, matriz e contador. No segundo, inicializa, para cada vértice i, o vizinho escolhido como inexistente e a menor distância como infinito. No terceiro, examina todos os pares (i, j) com i < j: ignora os pares com mcc, net ou area diferentes; calcula a distância de Haversine; ignora o par quando a distância excede a soma dos alcances; e, para cada extremo, atualiza o vizinho mais próximo quando a distância encontrada é estritamente menor que a menor registrada. No quarto passo, para cada vértice i com vizinho escolhido j, adiciona a aresta {i, j} se a escolha de j não for i ou se i < j.
 
Esse último critério evita a aresta duplicada quando a escolha é mútua, pois somente o menor índice a cria, e a inserção de arestas também rejeita duplicatas. Empates de distância mantêm o vizinho de menor índice, devido à comparação estrita. Como cada vértice propõe no máximo uma aresta, o número de arestas satisfaz E ≤ V, e vértices sem nenhum vizinho elegível permanecem isolados.
 
## Validação pela fórmula de Euler
 
Todo grafo simples planar com V ≥ 3 satisfaz E ≤ 3V - 6. A função de verificação devolve um de três resultados. Quando o grafo é nulo ou V < 3, o resultado é não aplicável. Quando E > 3V - 6, o resultado é não planar, com certeza. Quando E ≤ 3V - 6, o resultado é inconclusivo.
 
O teste de V < 3 é feito antes do cálculo de 3V - 6, porque o tipo size_t não tem sinal e os cálculos 3·1 - 6 e 3·2 - 6 sofreriam underflow. A condição é necessária, mas não suficiente: o grafo K3,3, com V = 6 e E = 9, satisfaz a desigualdade e não é planar, caso coberto por teste automatizado, enquanto o grafo K5, com V = 5 e E = 10 > 9, é corretamente detectado como não planar. O resultado é traduzido em mensagem textual por uma função própria.
 
## Algoritmo de interseção de segmentos
 
A função de orientação calcula o produto vetorial (b - a) × (c - a) de três pontos a, b e c, usando a longitude como abscissa e a latitude como ordenada, ou seja, (b.lon - a.lon)·(c.lat - a.lat) - (b.lat - a.lat)·(c.lon - a.lon). Um resultado positivo indica que c está à esquerda do segmento orientado de a para b, um resultado negativo indica que está à direita e o valor zero indica que os três pontos são colineares.
 
Dados dois segmentos AB e CD, calculam-se as orientações o1 = orientação(A, B, C), o2 = orientação(A, B, D), o3 = orientação(C, D, A) e o4 = orientação(C, D, B). Os segmentos se cruzam quando o1 e o2 têm sinais estritamente opostos e, simultaneamente, o3 e o4 têm sinais estritamente opostos. A comparação é feita pelos sinais, sem multiplicar os valores, o que evita estouro ou perda de precisão do produto. O teste usa quatro orientações e tem custo O(1) por par de segmentos.
 
Trata-se do cruzamento próprio: não são contados toques em extremos, sobreposição colinear nem vértice sobre o interior de outra aresta, pois todos esses casos geram orientação nula. Escalar os eixos, por exemplo multiplicando a longitude pelo cosseno da latitude, não altera o sinal das orientações, de modo que o resultado não depende dessa correção. O que importa é que os segmentos são tratados como retas no plano (longitude, latitude), e não como geodésicas.
 
## Análise de cruzamentos
 
A análise percorre todos os pares de arestas (i, j) com i < j. Para cada par, ignora-o se as arestas compartilham um vértice, pois por construção só se tocam no extremo comum, a menos que sejam colineares, caso não tratado. Caso contrário, aplica o teste de interseção de segmentos e, ao encontrar um cruzamento, devolve verdadeiro imediatamente. Se todos os pares são examinados sem cruzamento, devolve falso.
 
A função detecta, mas não conta nem lista os cruzamentos, pois para na primeira ocorrência. O melhor caso é O(1), quando o primeiro par examinado se cruza, e o pior caso é O(E²), quando não há cruzamento, que é justamente o caso de uma planta válida. O programa principal interpreta o resultado verdadeiro como a necessidade de isolamento ou de novas rotas e o resultado falso como ausência de cruzamentos na planta analisada.
 
Quanto à relação com a planaridade, se a análise termina sem cruzamentos, o desenho dado pelas coordenadas é plano, o que prova que o grafo é planar, supondo ausência de sobreposições colineares. Se um cruzamento é encontrado, isso mostra apenas que aquele desenho não é plano, e o grafo ainda poderia ser planar com outra disposição. Por isso o teste geométrico e o teste de Euler respondem a perguntas diferentes e se complementam.
 
## Complexidades de tempo
 
A inserção de vértice tem custo O(1) amortizado, acrescido das expansões da matriz, que somam O(V²) no total. A inserção de aresta custa O(deg(origem)), pois verifica duplicidade percorrendo a lista de adjacência, e em seguida O(1) amortizado. A consulta de adjacência pela lista custa O(deg(origem)), com pior caso O(Δ), enquanto a consulta pela matriz custa O(1). A obtenção da lista de vizinhos custa O(1), e percorrê-la custa O(deg(v)). A obtenção de segmento e a verificação de compartilhamento de vértice custam O(1).
 
A leitura do arquivo CSV custa O(n), em que n é o número de linhas lidas, mais o custo das inserções. A construção das conexões custa O(V²): compara todos os pares, calcula a distância de Haversine apenas para pares do mesmo mcc, net e area, insere as arestas em O(V·Δ) e limpa as estruturas em O(V + E + ordem²). A validação de Euler custa O(1). A análise de cruzamentos custa O(E²) no pior caso e O(1) no melhor; como E ≤ V na construção automática, o pior caso equivale a O(V²). A destruição do grafo custa O(V + E).
 
## Complexidades de espaço
 
A lista de adjacência ocupa O(V + E), isto é, ordem·8 + 2E·16 bytes. A matriz de adjacência ocupa O(V²), isto é, ordem² bytes, valor entre V² e 4V². O vetor de arestas ocupa O(E), com capacidade·16 bytes, e o vetor de vértices ocupa O(V), com capacidade·48 bytes. A construção das conexões usa espaço auxiliar O(V), correspondente a dois vetores de V elementos de 8 bytes. A validação de Euler e a análise de cruzamentos usam espaço O(1). O espaço total do grafo é, portanto, O(V²), dominado pela matriz de adjacência.
 
## Limitações
 
Quanto aos dados e ao modelo, as coordenadas do OpenCelliD são estimativas colaborativas e não a posição confirmada das torres, e os resultados geométricos herdam esse erro. Registros distintos com coordenadas idênticas, possíveis em dados reais, geram segmentos degenerados cujos casos colineares não são detectados. O alcance é uma estimativa e vale zero quando ausente, o que torna a antena praticamente inelegível. A regra de ligar cada antena apenas ao vizinho elegível mais próximo produz no máximo V arestas e não garante um grafo conexo.
 
Quanto às representações, a matriz usa O(V²) bytes, cerca de 1 GiB com 32.000 vértices e 4 GiB com 62.604, o que inviabiliza o conjunto completo em máquinas modestas. Como o grafo é esparso, com E ≤ V, a matriz desperdiça espaço e a lista é suficiente para todas as operações atuais. Manter matriz e listas em paralelo duplica a informação e o custo de atualização. A consulta pela lista custa O(deg) e a inserção de aresta herda esse custo. A expansão da matriz exige memória transitória de 1,25 vez a final e O(ordem²) de cópia. Os índices de vértices são size_t, mas a inserção de vértice devolve int, o que limita o identificador devolvido ao valor máximo de int. Não há remoção de vértices ou de arestas isoladas, apenas a reconstrução total.
 
Quanto à fórmula de Euler, ela é condição necessária e não suficiente, de modo que nunca confirma a planaridade, apenas a refuta. Para grafos criados pela construção automática, E ≤ V ≤ 3V - 6 quando V ≥ 3, então o resultado é sempre inconclusivo ou não aplicável, e o teste só discrimina grafos montados manualmente, como o K5 dos testes. Também não são usados limites mais fortes, como E ≤ 2V - 4 para grafos sem triângulos, nem algoritmos exatos de planaridade, como os baseados no teorema de Kuratowski ou o algoritmo de Hopcroft e Tarjan.
 
Quanto à interseção de segmentos e aos cruzamentos, a comparação de valores do tipo double com zero é exata, sem tolerância, e erros de arredondamento podem classificar casos quase colineares de forma inconsistente. Detecta-se apenas o cruzamento próprio, ignorando toques, sobreposição colinear e vértice sobre aresta. Os pontos são tratados como plano cartesiano e os segmentos como retas, enquanto a regra de conexão usa distância geodésica; a diferença é desprezível em regiões pequenas, mas cresce em arestas longas. A função devolve apenas verdadeiro ou falso e não informa quantos nem quais pares se cruzam. A comparação de todos os pares custa O(E²) e é o principal gargalo em conjuntos grandes; alternativas para fases futuras incluem filtro por caixa envolvente, divisão em células geográficas e varredura de linha, como o algoritmo de Shamos e Hoey, com custo O(E log E). Por fim, a análise é puramente geométrica e não considera relevo, vias ou obstáculos reais para o cabo.
 
## Cobertura de testes
 
Os testes automatizados cobrem a criação, a inserção e a contagem de vértices e arestas; a lista de adjacência, a consulta de adjacência, o acesso a vizinhos e a ausência de vizinhos; a construção de conexões elegíveis, as quantidades após a construção automática e o suporte a 1.000 vértices; os quatro resultados da validação de Euler; a detecção de cruzamento, a obtenção de segmento e o compartilhamento de vértice entre arestas; e a liberação de memória. Há lacunas: não existe caso sem cruzamento, com resultado falso, nem casos de toque em extremo, de colinearidade ou de vértices coincidentes.
 
## Referências
 
CORMEN, T. H. et al. Algoritmos: Teoria e Prática. Capítulo sobre Geometria Computacional (determinação de interseção de segmentos).
 
WEST, D. B. Introduction to Graph Theory. Planaridade e fórmula de Euler.
 
Unwired Labs. OpenCelliD: Open Database of Cell Towers. Disponível em: https://opencellid.org/.