param([switch]$ExigirCompleto)

$ErrorActionPreference = 'Stop'
$raizProjeto = Split-Path -Parent $PSScriptRoot
$executavel = Join-Path $raizProjeto 'build/grafo.exe'
$dataset = Join-Path $raizProjeto 'data/opencellid_brasil_filtrado.csv'
$pasta = Join-Path $raizProjeto ('results/integracao-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $pasta | Out-Null
$casos = [System.Collections.Generic.List[object]]::new()
$csv = Join-Path $pasta 'execucoes.csv'
$resumo = [ordered]@{
    issue = 35
    status = 'em_execucao'
    data_utc = [DateTime]::UtcNow.ToString('o')
    dataset_sha256 = (Get-FileHash -LiteralPath $dataset -Algorithm SHA256).Hash
    executavel_sha256 = (Get-FileHash -LiteralPath $executavel -Algorithm SHA256).Hash
    roteiro_sha256 = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash
    powershell = $PSVersionTable.PSVersion.ToString()
    sistema = [Environment]::OSVersion.ToString()
    casos = $casos
    pendencias = @(
        'Benchmark repetido da matriz: issue #24.',
        'Revisar os resultados completos da #17 apos a integracao na #34.'
    )
}

function Conferir([bool]$Condicao, [string]$Mensagem) {
    if (-not $Condicao) { throw $Mensagem }
}

function ExecutarCaso([string]$Nome, [string[]]$Argumentos, [int]$CodigoEsperado) {
    $stdout = Join-Path $pasta ($Nome + '.stdout.log')
    $stderr = Join-Path $pasta ($Nome + '.stderr.log')
    # Argumentos sao gerados pelo roteiro; aspas preservam espacos nos caminhos.
    $linhaArgumentos = ($Argumentos | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $processo = Start-Process -FilePath $executavel -ArgumentList $linhaArgumentos `
        -WorkingDirectory $raizProjeto -WindowStyle Hidden -PassThru -Wait `
        -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    $codigo = $processo.ExitCode
    $casos.Add([ordered]@{
        nome = $Nome; argumentos = $Argumentos; codigo = $codigo
        codigo_esperado = $CodigoEsperado; stdout = [IO.Path]::GetFileName($stdout)
        stderr = [IO.Path]::GetFileName($stderr)
    })
    Conferir ($codigo -eq $CodigoEsperado) "Codigo inesperado no caso $Nome`: $codigo."
    return (Get-Content -Raw -LiteralPath $stdout)
}

Push-Location $raizProjeto
try {
    $resumo['commit_base'] = (git rev-parse HEAD)
    $resumo['alteracoes_locais'] = @(git status --porcelain)
    $resumo['compilador'] = ((& gcc --version | Select-Object -First 1) -join '')
    # Referencia de regressao do dataset versionado; nao e prova dos algoritmos.
    $hashReferencia = '5EB50BD6954466F08ECA483B2D9DA18B50C297E54710532FB7D0646B2987F8DB'
    Conferir ($resumo.dataset_sha256 -eq $hashReferencia) 'Dataset mudou; revisar referencias documentadas da #35.'
    $arestasReferencia = @{ 1 = '0'; 100 = '33'; 1000 = '637' }
    $cruzamentosReferencia = @{ 1 = '0'; 100 = '0'; 1000 = '1' }
    foreach ($limite in @(1, 100, 1000, 1000)) {
        $nome = 'real-' + $limite + '-' + $casos.Count
        $saida = ExecutarCaso $nome @('--dataset', $dataset, '--limite', "$limite",
            '--estrutura', 'conjunta', '--saida', $csv) 0
        $linhas = @(Import-Csv -LiteralPath $csv)
        $r = $linhas[-1]
        Conferir ($linhas.Count -eq $casos.Count) 'CSV perdeu ou duplicou uma execucao.'
        Conferir ($r.vertices -eq "$limite" -and $r.limite -eq "$limite") 'Limite de vertices incorreto.'
        Conferir ($r.dataset -eq $dataset -and $r.estrutura -eq 'conjunta') 'Origem ou estrutura incorreta.'
        Conferir ($r.arestas -eq $arestasReferencia[$limite] -and
            $r.possui_cruzamentos -eq $cruzamentosReferencia[$limite] -and
            $r.registros_invalidos -eq '0') 'Resultado diverge da referencia do dataset real.'
        Conferir ($saida.Contains("Vertices: $($r.vertices)`r`n") -and
            $saida.Contains("Arestas: $($r.arestas)`r`n")) 'Terminal e CSV divergem nas contagens.'
        $v = [long]$r.vertices
        $e = [long]$r.arestas
        Conferir ($e -ge 0 -and $e -le ($v * ($v - 1) / 2)) 'Contagem invalida para grafo simples.'
        $eulerEsperado = if ($v -lt 3) { 'nao_aplicavel' } elseif ($e -gt 3 * $v - 6) { 'nao_planar' } else { 'inconclusivo' }
        Conferir ($r.euler -eq $eulerEsperado) 'Resultado de Euler inconsistente com V e E.'
        Conferir ($r.possui_cruzamentos -in @('0', '1')) 'Presenca de cruzamentos invalida.'
        $mensagem = if ($r.possui_cruzamentos -eq '1') { 'Foram detectados cruzamentos' } else { 'detector atual nao encontrou cruzamentos' }
        Conferir ($saida.Contains($mensagem)) 'Conclusao diverge da deteccao exportada.'
        Conferir ($saida.Contains('nao constitui um teste completo de planaridade')) 'Limitacao de Euler ausente.'
        $quantidadeEsperada = if ($limite -eq 1000) { '2' } else { '0' }
        Conferir ($r.quantidade_cruzamentos -eq $quantidadeEsperada -and
            $r.status_cruzamentos -eq 'calculado' -and
            $saida.Contains("Quantidade de cruzamentos: $quantidadeEsperada`r`n") -and
            $r.memoria_comum_bytes -match '^\d+$' -and
            $r.memoria_lista_total_bytes -match '^\d+$' -and
            $r.memoria_matriz_total_bytes -match '^\d+$' -and
            $r.status_memoria -eq 'estimada_modelo_alocacoes' -and
            $saida.Contains("Lista de adjacencia: $($r.memoria_lista_total_bytes)`r`n") -and
            $saida.Contains("Matriz de adjacencia: $($r.memoria_matriz_total_bytes)`r`n") -and
            $r.status_analise -eq 'parcial') 'Pendencias foram apresentadas como resultados completos.'
        foreach ($campo in @('leitura_cpu_ms', 'construcao_cpu_ms', 'euler_cpu_ms', 'cruzamentos_cpu_ms', 'total_cpu_ms')) {
            $tempo = [double]::Parse($r.$campo, [Globalization.CultureInfo]::InvariantCulture)
            Conferir ($tempo -ge 0 -and -not [double]::IsInfinity($tempo) -and
                -not [double]::IsNaN($tempo)) "Medicao invalida: $campo."
        }
        Conferir ((Get-Item (Join-Path $pasta ($nome + '.stderr.log'))).Length -eq 0) 'Erro inesperado na execucao.'
    }
    $linhas = @(Import-Csv -LiteralPath $csv)
    foreach ($campo in @('vertices', 'arestas', 'registros_invalidos', 'euler',
        'possui_cruzamentos', 'quantidade_cruzamentos', 'memoria_comum_bytes',
        'memoria_lista_total_bytes', 'memoria_matriz_total_bytes')) {
        Conferir ($linhas[2].$campo -eq $linhas[3].$campo) "Repeticao produziu resultado diferente: $campo."
    }
    foreach ($estrutura in @('lista', 'matriz')) {
        $subset = Join-Path $raizProjeto 'data/subconjuntos/opencellid_n100.csv'
        $saidaEstrutura = ExecutarCaso ('estrutura-' + $estrutura) @('--dataset', $subset,
            '--limite', '0', '--estrutura', $estrutura, '--saida', $csv) 0
        $rEstrutura = (Import-Csv -LiteralPath $csv | Select-Object -Last 1)
        Conferir ($rEstrutura.estrutura -eq $estrutura -and $rEstrutura.vertices -eq '100' -and
            $rEstrutura.arestas -eq '2' -and $rEstrutura.registros_invalidos -eq '0' -and
            $rEstrutura.total_cpu_ms -match '^\d+\.\d{3}$') "Modo $estrutura incorreto."
        if ($estrutura -eq 'lista') {
            Conferir ([long]$rEstrutura.memoria_lista_total_bytes -gt 0 -and
                $rEstrutura.memoria_matriz_total_bytes -eq '0') 'Modo lista alocou matriz.'
        } else {
            Conferir ($rEstrutura.memoria_lista_total_bytes -eq '0' -and
                [long]$rEstrutura.memoria_matriz_total_bytes -gt 0) 'Modo matriz alocou lista.'
        }
        Conferir ($saidaEstrutura.Contains("Estrutura: $estrutura de adjacencia")) "Terminal nao identificou $estrutura."
    }
    Conferir (@(Import-Csv -LiteralPath $csv).Count -eq 6) 'CSV divergiu nas execucoes de representacao.'
    Conferir ((Get-FileHash -LiteralPath $dataset -Algorithm SHA256).Hash -eq $resumo.dataset_sha256) 'Dataset foi alterado.'
    $resumo['resultados'] = $linhas
    $resumo.status = 'parcial_validado'
    Write-Output 'Integracao disponivel validada com dataset real. A #35 continua parcial.'
} catch {
    $resumo.status = 'falhou'
    $resumo['erro'] = $_.Exception.Message
    throw
} finally {
    $resumo | ConvertTo-Json -Depth 8 | Set-Content -Encoding utf8 (Join-Path $pasta 'resumo.json')
    Write-Output "Evidencias: $pasta"
    Pop-Location
}
if ($ExigirCompleto) {
    Write-Output 'Validacao completa bloqueada pelas pendencias registradas. Issue #35 nao concluida.'
    exit 2
}
