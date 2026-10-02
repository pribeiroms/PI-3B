param(
    [int]$Repeticoes = 5,
    [string]$Saida = 'results/benchmark_matriz.csv'
)

$ErrorActionPreference = 'Stop'
if ($Repeticoes -lt 1) { throw 'Repeticoes deve ser maior que zero.' }
$raizProjeto = Split-Path -Parent $PSScriptRoot
$executavel = Join-Path $raizProjeto 'build/grafo.exe'
$pastaBuild = Join-Path $raizProjeto 'build'
$destino = Join-Path $raizProjeto $Saida
$idExecucao = [guid]::NewGuid().ToString('N')
$csvInterno = Join-Path $pastaBuild ('benchmark-matriz-interno-' + $idExecucao + '.csv')
$referencias = @(
    @{ n = 100; arestas = 2; cruzamentos = 0 },
    @{ n = 500; arestas = 48; cruzamentos = 0 },
    @{ n = 1000; arestas = 186; cruzamentos = 7 },
    @{ n = 5000; arestas = 1897; cruzamentos = 225 }
)

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
        $dataset = "data/subconjuntos/opencellid_n$n.csv"
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
            Write-Output "Matriz N=$n repeticao=$repeticao : V=$($resultado.vertices), E=$($resultado.arestas), CPU=$($resultado.total_cpu_ms) ms, parede=$($linha.tempo_parede_total_ms) ms"
        }
    }
    $registros | Export-Csv -LiteralPath $destino -NoTypeInformation -Encoding UTF8
    Write-Output "Benchmark salvo em $destino ($($registros.Count) execucoes)."
} finally {
    Pop-Location
}