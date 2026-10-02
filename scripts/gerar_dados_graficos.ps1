param(
    [string]$ListaCsv = 'results/benchmark_lista.csv',
    [string]$MatrizCsv = 'results/benchmark_matriz.csv',
    [string]$PastaSaida = 'results/graficos'
)

$ErrorActionPreference = 'Stop'
$raizProjeto = Split-Path -Parent $PSScriptRoot
$caminhoLista = Join-Path $raizProjeto $ListaCsv
$caminhoMatriz = Join-Path $raizProjeto $MatrizCsv
$pastaSaida = Join-Path $raizProjeto $PastaSaida

if (-not (Test-Path -LiteralPath $caminhoLista)) {
    throw "Nao encontrado: $caminhoLista. Rode scripts/benchmark_lista.ps1 antes."
}
if (-not (Test-Path -LiteralPath $caminhoMatriz)) {
    throw "Nao encontrado: $caminhoMatriz. Rode scripts/benchmark_matriz.ps1 antes."
}

New-Item -ItemType Directory -Force -Path $pastaSaida | Out-Null

$dadosLista = Import-Csv -LiteralPath $caminhoLista
$dadosMatriz = Import-Csv -LiteralPath $caminhoMatriz

function Media-Por-Grupo {
    param($Dados, [string]$CampoAgrupamento, [string]$CampoValor)
    $Dados | Group-Object -Property $CampoAgrupamento | ForEach-Object {
        $valores = $_.Group | ForEach-Object {
            [double]::Parse($_.$CampoValor, [Globalization.CultureInfo]::InvariantCulture)
        }
        [pscustomobject]@{
            Chave = $_.Name
            Media = ($valores | Measure-Object -Average).Average
            Repeticoes = $_.Group.Count
        }
    }
}

# 1) Tempo (ms) x numero de vertices, por estrutura
$temposLista = Media-Por-Grupo -Dados $dadosLista -CampoAgrupamento 'vertices' -CampoValor 'total_cpu_ms'
$temposMatriz = Media-Por-Grupo -Dados $dadosMatriz -CampoAgrupamento 'vertices' -CampoValor 'total_cpu_ms'

$tempoVsVertices = @()
$tempoVsVertices += $temposLista | ForEach-Object {
    [pscustomobject]@{
        vertices = $_.Chave
        estrutura = 'lista'
        tempo_medio_cpu_ms = [string]::Format([Globalization.CultureInfo]::InvariantCulture, '{0:F3}', $_.Media)
        repeticoes = $_.Repeticoes
    }
}
$tempoVsVertices += $temposMatriz | ForEach-Object {
    [pscustomobject]@{
        vertices = $_.Chave
        estrutura = 'matriz'
        tempo_medio_cpu_ms = [string]::Format([Globalization.CultureInfo]::InvariantCulture, '{0:F3}', $_.Media)
        repeticoes = $_.Repeticoes
    }
}
$tempoVsVertices | Sort-Object { [int]$_.vertices }, estrutura |
    Export-Csv -LiteralPath (Join-Path $pastaSaida 'tempo_vs_vertices.csv') -NoTypeInformation -Encoding UTF8

# 2) Memoria (bytes) x numero de vertices, por estrutura
$memoriaVsVertices = @()
$memoriaVsVertices += $dadosLista | Group-Object -Property 'vertices' | ForEach-Object {
    [pscustomobject]@{
        vertices = $_.Name
        estrutura = 'lista'
        memoria_bytes = ($_.Group | Select-Object -First 1).memoria_lista_total_bytes
    }
}
$memoriaVsVertices += $dadosMatriz | Group-Object -Property 'vertices' | ForEach-Object {
    [pscustomobject]@{
        vertices = $_.Name
        estrutura = 'matriz'
        memoria_bytes = ($_.Group | Select-Object -First 1).memoria_matriz_total_bytes
    }
}
$memoriaVsVertices | Sort-Object { [int]$_.vertices }, estrutura |
    Export-Csv -LiteralPath (Join-Path $pastaSaida 'memoria_vs_vertices.csv') -NoTypeInformation -Encoding UTF8

# 3) Lista x Matriz, lado a lado por numero de vertices
$listaVsMatriz = $tempoVsVertices | Where-Object { $_.estrutura -eq 'lista' } | ForEach-Object {
    $vertices = $_.vertices
    $tempoMatrizLinha = $tempoVsVertices | Where-Object { $_.estrutura -eq 'matriz' -and $_.vertices -eq $vertices }
    $memLista = ($memoriaVsVertices | Where-Object { $_.estrutura -eq 'lista' -and $_.vertices -eq $vertices }).memoria_bytes
    $memMatriz = ($memoriaVsVertices | Where-Object { $_.estrutura -eq 'matriz' -and $_.vertices -eq $vertices }).memoria_bytes
    [pscustomobject]@{
        vertices = $vertices
        tempo_lista_cpu_ms = $_.tempo_medio_cpu_ms
        tempo_matriz_cpu_ms = $tempoMatrizLinha.tempo_medio_cpu_ms
        memoria_lista_bytes = $memLista
        memoria_matriz_bytes = $memMatriz
    }
}
$listaVsMatriz | Sort-Object { [int]$_.vertices } |
    Export-Csv -LiteralPath (Join-Path $pastaSaida 'lista_vs_matriz.csv') -NoTypeInformation -Encoding UTF8

# 4) Numero de arestas x tempo (ms), por estrutura
$arestasVsTempo = @()
$arestasVsTempo += (Media-Por-Grupo -Dados $dadosLista -CampoAgrupamento 'arestas' -CampoValor 'total_cpu_ms') | ForEach-Object {
    [pscustomobject]@{
        arestas = $_.Chave
        estrutura = 'lista'
        tempo_medio_cpu_ms = [string]::Format([Globalization.CultureInfo]::InvariantCulture, '{0:F3}', $_.Media)
    }
}
$arestasVsTempo += (Media-Por-Grupo -Dados $dadosMatriz -CampoAgrupamento 'arestas' -CampoValor 'total_cpu_ms') | ForEach-Object {
    [pscustomobject]@{
        arestas = $_.Chave
        estrutura = 'matriz'
        tempo_medio_cpu_ms = [string]::Format([Globalization.CultureInfo]::InvariantCulture, '{0:F3}', $_.Media)
    }
}
$arestasVsTempo | Sort-Object { [int]$_.arestas }, estrutura |
    Export-Csv -LiteralPath (Join-Path $pastaSaida 'arestas_vs_tempo.csv') -NoTypeInformation -Encoding UTF8

Write-Output "Dados para graficos gerados em: $pastaSaida"