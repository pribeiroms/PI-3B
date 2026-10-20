param(
    [int]$Repeticoes = 5,
    [string]$Saida = '',
    # fase1: amostra uniforme do Brasil, usada na Fase I.
    # fase2: recorte da regiao mais densa, usado nos experimentos da Fase II.
    # Os dois conjuntos convivem; a escolha nao altera nenhum arquivo do outro.
    [ValidateSet('fase1', 'fase2')]
    [string]$Subconjuntos = 'fase1'
)

$ErrorActionPreference = 'Stop'
if ($Repeticoes -lt 1) { throw 'Repeticoes deve ser maior que zero.' }
$raizProjeto = Split-Path -Parent $PSScriptRoot
$executavel = Join-Path $raizProjeto 'build/grafo.exe'
$pastaBuild = Join-Path $raizProjeto 'build'
if ([string]::IsNullOrEmpty($Saida)) {
    $Saida = if ($Subconjuntos -eq 'fase1') { 'results/benchmark_matriz.csv' }
             else { 'results/benchmark_matriz_fase2.csv' }
}
$destino = Join-Path $raizProjeto $Saida
$idExecucao = [guid]::NewGuid().ToString('N')
$csvInterno = Join-Path $pastaBuild ('benchmark-matriz-interno-' + $idExecucao + '.csv')
# Os valores de referencia sao os medidos para cada conjunto. Eles existem para
# o benchmark falhar alto se o grafo mudar sem querer, em vez de publicar
# tempos de um grafo diferente do esperado.
if ($Subconjuntos -eq 'fase1') {
    $modelo = 'data/subconjuntos/opencellid_n{0}.csv'
    $referencias = @(
        @{ n = 100; arestas = 2; cruzamentos = 0 },
        @{ n = 500; arestas = 48; cruzamentos = 0 },
        @{ n = 1000; arestas = 186; cruzamentos = 7 },
        @{ n = 5000; arestas = 1897; cruzamentos = 225 }
    )
} else {
    $modelo = 'data/subconjuntos/fase2/opencellid_denso_n{0}.csv'
    $referencias = @(
        @{ n = 100; arestas = 66; cruzamentos = 10 },
        @{ n = 500; arestas = 336; cruzamentos = 124 },
        @{ n = 1000; arestas = 686; cruzamentos = 252 },
        @{ n = 5000; arestas = 3446; cruzamentos = 1725 }
    )
}

Push-Location $raizProjeto
try {
    if (-not (Test-Path -LiteralPath $executavel)) { throw 'Execute mingw32-make antes do benchmark.' }
    if (Test-Path -LiteralPath $destino) { throw "O arquivo ja existe; escolha outro -Saida para preservar os dados: $destino" }
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destino) | Out-Null
    $registros = [System.Collections.Generic.List[object]]::new()
    $compilador = ((& gcc --version | Select-Object -First 1) -join '').Trim()
    $processador = $env:PROCESSOR_IDENTIFIER
    foreach ($referencia in $referencias) {
        $n = $referencia.n
        $dataset = $modelo -f $n
        for ($repeticao = 1; $repeticao -le $Repeticoes; $repeticao++) {
            $stdout = Join-Path $pastaBuild "benchmark-matriz-$idExecucao-$n-$repeticao.stdout.log"
            $stderr = Join-Path $pastaBuild "benchmark-matriz-$idExecucao-$n-$repeticao.stderr.log"
            $argumentos = '--dataset "{0}" --limite 0 --estrutura matriz --saida "{1}"' -f $dataset, $csvInterno
            $cronometro = [Diagnostics.Stopwatch]::StartNew()
            $processo = Start-Process -FilePath $executavel -ArgumentList $argumentos `
                -WorkingDirectory $raizProjeto -WindowStyle Hidden -PassThru -Wait `
                -RedirectStandardOutput $stdout -RedirectStandardError $stderr
            $cronometro.Stop()
            if ($processo.ExitCode -ne 0) {
                $erro = Get-Content -Raw -LiteralPath $stderr
                throw "Benchmark falhou em N=$n repeticao=$repeticao : $erro"
            }
            $resultado = Import-Csv -LiteralPath $csvInterno | Select-Object -Last 1
            if ($resultado.estrutura -ne 'matriz' -or $resultado.vertices -ne "$n" -or
                $resultado.arestas -ne "$($referencia.arestas)" -or
                $resultado.euler -ne 'inconclusivo' -or
                $resultado.quantidade_cruzamentos -ne "$($referencia.cruzamentos)" -or
                $resultado.registros_invalidos -ne '0' -or
                [double]::Parse($resultado.total_cpu_ms,
                    [Globalization.CultureInfo]::InvariantCulture) -lt 0 -or
                $resultado.memoria_matriz_total_bytes -notmatch '^\d+$' -or
                $resultado.memoria_lista_total_bytes -ne '0') {
                throw "Resultado ou representacao divergente em N=$n repeticao=$repeticao."
            }
            $linha = [ordered]@{
                benchmark_id = $idExecucao
                dataset_n = $n
                repeticao = $repeticao
                tempo_parede_total_ms = [string]::Format(
                    [Globalization.CultureInfo]::InvariantCulture, '{0:F3}',
                    $cronometro.Elapsed.TotalMilliseconds)
                compilador = $compilador
                sistema = [Environment]::OSVersion.ToString()
                processador = $processador
            }
            foreach ($campo in $resultado.PSObject.Properties) { $linha[$campo.Name] = $campo.Value }
            $registros.Add([pscustomobject]$linha)
            Write-Output "Matriz [$Subconjuntos] N=$n repeticao=$repeticao : V=$($resultado.vertices), E=$($resultado.arestas), CPU=$($resultado.total_cpu_ms) ms, parede=$($linha.tempo_parede_total_ms) ms"
        }
    }
    $registros | Export-Csv -LiteralPath $destino -NoTypeInformation -Encoding UTF8
    Write-Output "Benchmark salvo em $destino ($($registros.Count) execucoes)."
} finally {
    Pop-Location
}