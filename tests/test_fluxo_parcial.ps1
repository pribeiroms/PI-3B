# Verifica somente o fluxo disponivel da #34; nao substitui a #35.
$ErrorActionPreference = 'Stop'
$raizProjeto = Split-Path -Parent $PSScriptRoot
$executavel = Join-Path $raizProjeto 'build/grafo.exe'
$pastaTeste = Join-Path $raizProjeto ('build/fluxo-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $pastaTeste | Out-Null

function Executar([string[]]$Argumentos, [int]$Esperado = 0) {
    $ErrorActionPreference = 'Continue'
    $texto = & $executavel @Argumentos 2> (Join-Path $pastaTeste 'stderr.txt')
    $codigo = $LASTEXITCODE
    if ($codigo -ne $Esperado) {
        throw "Codigo $codigo, esperado $Esperado. Argumentos: $Argumentos"
    }
    return ($texto -join "`n")
}

Push-Location $raizProjeto
try {
    $csv = Join-Path $pastaTeste 'resultados.csv'
    $real = Join-Path $raizProjeto 'data/opencellid_brasil_filtrado.csv'
    $saida = Executar @('--dataset', $real, '--limite', '100', '--estrutura', 'conjunta', '--saida', $csv)
    if ($saida -notmatch 'Vertices: 100' -or $saida -notmatch 'pendente da #19') {
        throw 'Saida nao apresenta os resultados e as pendencias esperadas.'
    }
    $null = Executar @('--dataset', $real, '--limite', '10', '--saida', $csv)
    $linhas = @(Import-Csv -LiteralPath $csv)
    if ($linhas.Count -ne 2 -or $linhas[0].vertices -ne '100' -or $linhas[1].vertices -ne '10') {
        throw 'CSV nao preservou as duas execucoes ou seus limites.'
    }
    foreach ($linha in $linhas) {
        if ($linha.estrutura -ne 'conjunta' -or $linha.memoria_bytes -ne '' -or
            $linha.quantidade_cruzamentos -ne '' -or $linha.status_analise -ne 'parcial') {
            throw 'CSV atribuiu valores a resultados ainda indisponiveis.'
        }
        if ($linha.dataset -ne $real -or $linha.euler -ne 'inconclusivo' -or
            $linha.status_memoria -ne 'pendente_19' -or $linha.status_cruzamentos -ne 'pendente_16') {
            throw 'Metadados incorretos no CSV.'
        }
        foreach ($campo in @('leitura_cpu_ms', 'construcao_cpu_ms', 'euler_cpu_ms', 'cruzamentos_cpu_ms')) {
            $valor = [double]::Parse($linha.$campo, [Globalization.CultureInfo]::InvariantCulture)
            if ($valor -lt 0) { throw 'Tempo indisponivel durante teste.' }
        }
    }
    foreach ($limite in @('-1', 'abc', '1.5', '10abc', '184467440737095516160')) {
        $null = Executar @('--limite', $limite, '--saida', $csv) 2
    }
    foreach ($estrutura in @('lista', 'matriz', 'invalida')) {
        $null = Executar @('--estrutura', $estrutura, '--saida', $csv) 2
    }
    $null = Executar @('--limite') 2
    $null = Executar @('--limite', '1', '--limite', '2') 2
    $null = Executar @('--desconhecida', '1') 2
    $null = Executar @('--dataset', (Join-Path $pastaTeste 'inexistente.csv'), '--saida', $csv) 1
    $null = Executar @('--limite', '1', '--saida', (Join-Path $pastaTeste 'inexistente/saida.csv')) 1
    $ajuda = Executar @('--help')
    if ($ajuda -notmatch 'Uso:') { throw 'Ajuda ausente.' }
    if (@(Import-Csv -LiteralPath $csv).Count -ne 2) { throw 'Falhas alteraram resultados existentes.' }

    $fixture = Join-Path $pastaTeste 'antenas, exemplo.csv'
    @(
        'radio,mcc,net,area,cell,lat,lon,range,samples',
        'GSM,724,2,10,1,0,0,1000,1',
        'registro invalido',
        'GSM,724,2,10,2,0,0.001,1000,1'
    ) | Set-Content -Encoding ascii -LiteralPath $fixture
    $null = Executar @('--dataset', $fixture, '--limite', '0', '--saida', $csv)
    $linhas = @(Import-Csv -LiteralPath $csv)
    $ultima = $linhas[-1]
    if ($ultima.vertices -ne '2' -or $ultima.arestas -ne '1' -or
        $ultima.registros_invalidos -ne '1' -or $ultima.dataset -ne $fixture -or
        $ultima.euler -ne 'nao_aplicavel' -or $ultima.possui_cruzamentos -ne '0') {
        throw 'Falha no limite zero, escape CSV ou dados da execucao.'
    }
    $antes = Get-Content -Raw -LiteralPath $fixture
    $null = Executar @('--dataset', $fixture, '--saida', $fixture) 1
    if ((Get-Content -Raw -LiteralPath $fixture) -ne $antes) { throw 'Dataset alterado pela exportacao.' }
    $vazio = Join-Path $pastaTeste 'vazio.csv'
    'radio,mcc,net,area,cell,lat,lon,range,samples' | Set-Content -Encoding ascii $vazio
    $null = Executar @('--dataset', $vazio, '--saida', $csv) 1
    if (@(Import-Csv -LiteralPath $csv).Count -ne 3) { throw 'Execucao vazia exportada.' }

    # Isola a saida padrao para testar compatibilidade com argumentos posicionais.
    New-Item -ItemType Directory -Path (Join-Path $pastaTeste 'results') | Out-Null
    Push-Location $pastaTeste
    try {
        $null = Executar @($fixture, '1')
        $posicional = @(Import-Csv 'results/execucoes.csv')
        if ($posicional.Count -ne 1 -or $posicional[0].vertices -ne '1') {
            throw 'Execucao posicional ou saida padrao incorreta.'
        }
    } finally { Pop-Location }
    Write-Output 'Testes do fluxo parcial da #34 passaram.'
} finally { Pop-Location }
