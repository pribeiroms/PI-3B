# Transformação do problema real em grafo

## Objetivo e contexto

Este documento formaliza a abstração usada na Fase I do projeto para analisar, em uma representação geográfica simplificada, possíveis conexões entre células de uma rede de telecomunicações. O cenário considera pontos de rede móvel distribuídos pelo território brasileiro e pergunta se uma seleção de ligações candidatas pode ser representada sem cruzamentos geométricos entre seus segmentos.

A abstração serve para explorar uma questão preliminar de planejamento. Ela não descreve a infraestrutura física existente: o OpenCelliD registra posições estimadas de células, e o programa não consulta cadastro de dutos, postes, vias, servidões ou cabos instalados. Assim, os resultados são hipóteses geométricas para estudo, e não um projeto executivo de engenharia.

## Definição formal

O modelo é um grafo simples, não direcionado e sem peso, denotado por `G = (V, E)`, associado a uma representação geométrica no plano.

- `V` é o conjunto de registros válidos do dataset carregados na execução. Cada vértice representa uma célula/estação identificada por `radio`, `mcc`, `net`, `area` e `cell`. O identificador interno do vértice é atribuído pela ordem de carregamento. O carregador não deduplica registros por essa chave; portanto, cada linha válida carregada gera um vértice.
- `E` é o conjunto de pares não ordenados de vértices distintos aceitos pela regra de conexão. Uma aresta representa uma ligação candidata entre dois pontos, interpretada no cenário do trabalho como uma possível rota de cabo subterrâneo.
- As arestas não têm peso na implementação da Fase I. A distância e os alcances são usados para decidir elegibilidade e vizinhança, não armazenados como custo da aresta.

Para cada vértice `v`, os atributos relevantes são os identificadores de país e rede (`mcc`, `net`), a área (`area`), as coordenadas geográficas (`lat`, `lon`) e o alcance estimado (`range`). `radio`, `cell` e `samples` identificam ou descrevem o registro, mas não participam da regra atual de construção das arestas. Em particular, a tecnologia não é usada para separar as conexões.

## Origem dos dados

O arquivo utilizado é `data/opencellid_brasil_filtrado.csv`, um recorte brasileiro da base colaborativa OpenCelliD. A documentação do dataset registra 62.604 linhas de dados e as tecnologias GSM, UMTS, LTE e NR/5G. O formato tem nove colunas: `radio`, `mcc`, `net`, `area`, `cell`, `lat`, `lon`, `range` e `samples`. A origem, os campos e as limitações de rastreabilidade estão descritos em [DATASET.md](DATASET.md); a fonte é [OpenCelliD](https://opencellid.org/) e sua [documentação](https://docs.opencellid.org/).

As coordenadas são estimativas derivadas de observações reportadas, não uma confirmação da posição física de torres ou de pontos de passagem de cabos. A quantidade efetivamente analisada depende do limite informado na execução. Por padrão, o programa carrega até os primeiros 1.000 registros válidos; o limite `0` solicita a leitura de todos os registros.

## Uso das coordenadas e regra de conexão

As coordenadas `lat` e `lon`, em graus decimais, posicionam os vértices e são usadas para estimar a distância entre pares. A implementação calcula a distância geodésica aproximada pela fórmula de Haversine, usando raio terrestre de 6.371.000 metros. Para representar cruzamentos, trata cada aresta como o segmento reto entre as coordenadas dos seus extremos, usando longitude e latitude como coordenadas planas.

Dois vértices distintos `u` e `v` são candidatos à conexão quando satisfazem simultaneamente:

1. `u.mcc = v.mcc`;
2. `u.net = v.net`;
3. `u.area = v.area`;
4. `d(u,v) <= u.range + v.range`, em que `d` é a distância calculada em metros.

Entre os candidatos elegíveis de cada vértice, o programa seleciona o vizinho de menor distância. A aresta não direcionada `{u,v}` é incluída se `u` selecionar `v` ou `v` selecionar `u`. Empates permanecem com o primeiro candidato encontrado na ordem de leitura/comparação. Dessa maneira, o resultado corresponde à união das escolhas de vizinho mais próximo, não a todas as combinações elegíveis nem necessariamente a um emparelhamento recíproco.

## Planaridade e interpretação do cabeamento subterrâneo

No modelo geométrico adotado, um cruzamento ocorre quando dois segmentos de aresta sem vértice comum se interceptam propriamente no plano. A rotina atual verifica essa condição por orientação dos segmentos. Cruzamentos são interpretados como pontos em que a planta hipotética poderia exigir isolamento, mudança de rota ou outra solução de engenharia.

Essa verificação é apenas um indicador inicial relacionado à planaridade. Um grafo planar admite algum desenho sem cruzamentos, enquanto o programa verifica cruzamentos em um desenho específico, determinado pelas coordenadas observadas e pelas arestas escolhidas. Além disso, o teste atual considera cruzamentos próprios e não classifica como cruzamento segmentos colineares ou contatos apenas nas extremidades. Coordenadas geográficas são aproximadas como plano para essa etapa, sem projeção cartográfica local.

Na infraestrutura subterrânea real, dois cabos podem se cruzar em planta e passar em profundidades diferentes, ou compartilhar travessias protegidas; também podem ser inviáveis mesmo sem cruzamento geométrico por causa de solo, vias, propriedade, dutos existentes, custo ou normas. O modelo não representa profundidade, obstáculos, capacidade, custo, redundância ou conectividade ponta a ponta. Portanto, ausência de cruzamentos no resultado não prova que a instalação seja viável, e presença de cruzamentos não prova que seja impossível. A planaridade funciona aqui como aproximação para sinalizar conflitos potenciais de traçado.

## Pergunta de negócio analisada

> Considerando as posições estimadas das células e as conexões candidatas entre pontos da mesma área e rede que estejam dentro do alcance combinado, a seleção de vizinhos mais próximos produz segmentos sem cruzamentos geométricos, ou indica pontos que merecem revisão preliminar de rota para uma possível implantação subterrânea?

A resposta do programa é restrita à amostra e à regra acima: informa se encontrou pelo menos um cruzamento próprio entre arestas selecionadas. Ela não estima investimento, cobertura, qualidade de serviço ou viabilidade de implantação.

## Procedimento reproduzível

Na raiz do projeto, o fluxo padrão é:

```sh
mingw32-make run
```

Para escolher arquivo e limite explicitamente:

```sh
build/grafo.exe data/opencellid_brasil_filtrado.csv 1000
```

Use `0` como limite para carregar todos os registros válidos. A rotina de cruzamentos compara pares de arestas; por isso, o custo cresce quadraticamente com o número de arestas. Para análises maiores, convém documentar recorte geográfico e filtros aplicados, mantendo o CSV original intacto.

## Limites para uso em artigo científico

Uma análise ou artigo deve informar pelo menos: versão/data do arquivo e sua procedência; limite e critérios de registros carregados; número de vértices e arestas obtidos; distribuição geográfica da amostra; regra de distância e seleção de vizinhos; método de detecção de cruzamentos; e limitações da representação. A documentação atual especifica o método implementado, mas resultados quantitativos devem ser medidos em cada execução e registrados com a respectiva amostra. A interpretação deve manter explícita a diferença entre posição estimada de célula e localização/rota real de infraestrutura.

## Validação de planaridade (Euler)

Com base na fórmula de Euler, todo grafo simples planar com V >= 3 satisfaz:

`E <= 3V - 6`

onde V é o número de vértices e E é o número de arestas.

Essa desigualdade é usada para validar a planaridade do grafo, mas é uma condição **necessária, não suficiente**.

**Violada** (E > 3V - 6): o grafo é certamente **não planar**.

**Aceita** (E <= 3V - 6): o resultado é **inconclusivo**. A análise baseada apenas em Euler não garante que o grafo seja planar (contraexemplo: K3,3, com V = 6 e E = 9, que satisfaz a condição mas não é planar).

**V < 3**: a condição não se aplica.
