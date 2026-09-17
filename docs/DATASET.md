# Dataset selecionado — OpenCelliD Brasil

## Arquivo utilizado

- Caminho no projeto: `data/opencellid_brasil_filtrado.csv`
- Formato: CSV, com cabeçalho e valores decimais separados por vírgula.
- Registros: 62.604; todos possuem identificador de célula, tecnologia e coordenadas
  geográficas válidas.
- Tecnologias presentes: GSM (1.651), UMTS (6.326), LTE (53.830) e NR/5G (797).

## Origem e rastreabilidade

O arquivo é um recorte brasileiro da base [OpenCelliD](https://opencellid.org/),
uma base aberta e colaborativa de observações de redes celulares, mantida pela
Unwired Labs. A plataforma disponibiliza exportações CSV por país e informa que
as posições são estimativas derivadas de observações com GPS; portanto, elas não
devem ser interpretadas como a localização física confirmada das torres.

A rastreabilidade no repositório é mantida pelo nome do arquivo e por este
documento. A fonte e as condições de atribuição/licença devem ser verificadas na
[documentação oficial do OpenCelliD](https://docs.opencellid.org/).

## Estrutura dos dados

| Campo | Uso no projeto |
| --- | --- |
| `radio` | Tecnologia celular (GSM, UMTS, LTE ou NR). |
| `mcc`, `net` | País e operadora móvel. |
| `area`, `cell` | Identificadores da área e da célula; com `radio`, `mcc` e `net`, formam a chave da célula. |
| `lat`, `lon` | Latitude e longitude em graus decimais. |
| `range` | Alcance estimado, em metros, quando disponível. |
| `samples` | Número de observações que contribuíram para a estimativa. |

## Justificativa técnica

O dataset atende aos critérios definidos para o trabalho:

- **Volume:** são 62.604 registros utilizáveis, muito acima do mínimo de 1.000.
- **Georreferenciamento:** cada registro possui `lat` e `lon`, permitindo calcular
  distância entre células e representar a rede espacialmente.
- **Qualidade:** há tecnologia, identificadores de rede/célula e os campos
  `samples` e `range`, que permitem filtrar ou ponderar análises por confiança e
  alcance. Coordenadas são estimadas por uma fonte colaborativa, uma limitação que
  será considerada na interpretação dos resultados.
- **Aplicabilidade:** descreve células de redes móveis brasileiras, incluindo LTE e
  NR/5G, diretamente pertinentes a um cenário de telecomunicações.
- **Processamento em C:** é um CSV simples, com nove colunas escalares. Pode ser
  lido sequencialmente com `fgets` e `sscanf`/`strtok`, sem dependências externas.
- **Construção do grafo:** cada célula pode ser um vértice. Arestas podem conectar
  células da mesma operadora/tecnologia dentro de um limiar de distância, ou cujas
  áreas de cobertura estimada se interceptam. `range`, `samples` e a distância podem
  compor os pesos das arestas.

## Reprodutibilidade

O programa deve receber o caminho `data/opencellid_brasil_filtrado.csv`
como entrada e validar cabeçalho, conversão numérica e coordenadas antes de criar
os vértices. Para experimentos menores, filtros por `radio`, operadora ou região
podem ser aplicados durante a leitura, preservando o arquivo original sem edição.
