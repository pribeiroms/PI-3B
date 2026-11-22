param(
    [ValidateSet('lista', 'matriz')]
    [string]$Estrutura = 'lista',
    [int]$Repeticoes = 5,
    [string]$Saida = ''
)

$ErrorActionPreference = 'Stop'
if ($Repeticoes -lt 1) { throw 'Repeticoes deve ser maior que zero.' }
$raizProjeto = Split-Path -Parent $PSScriptRoot
$executavel = Join-Path $raizProjeto 'build/grafo.exe'
if ([string]::IsNullOrEmpty($Saida)) { $Saida = "results/agm_$Estrutura.csv" }
$destino = Join-Path $raizProjeto $Saida
$idExecucao = [guid]::NewGuid().ToString('N')
$tamanhos = @(10, 20, 30, 40, 100, 500, 1000, 5000)
$cult = [Globalization.CultureInfo]::InvariantCulture

Push-Location $raizProjeto
try {
    if (-not (Test-Path -LiteralPath $executavel)) { throw 'Execute mingw32-make antes do benchmark.' }
    if (Test-Path -LiteralPath $destino) { throw "O arquivo ja existe; escolha outro -Saida: $destino" }
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destino) | Out-Null
    $registros = [System.Collections.Generic.List[object]]::new()
    $compilador = ((& gcc --version | Select-Object -First 1) -join '').Trim()
    $processador = $env:PROCESSOR_IDENTIFIER
    foreach ($n in $tamanhos) {
        $dataset = "data/subconjuntos/fase2/opencellid_denso_n$n.csv"
        if (-not (Test-Path -LiteralPath $dataset)) { throw "Subconjunto ausente: $dataset" }
        for ($rep = 1; $rep -le $Repeticoes; $rep++) {
            $stdout = Join-Path $raizProjeto "build/agm-$idExecucao-$Estrutura-$n-$rep.stdout.log"
            $stderr = Join-Path $raizProjeto "build/agm-$idExecucao-$Estrutura-$n-$rep.stderr.log"
            $args = '--modo agm --dataset "{0}" --limite 0 --estrutura {1}' -f $dataset, $Estrutura
            $timer = [Diagnostics.Stopwatch]::StartNew()
            $processo = Start-Process -FilePath $executavel -ArgumentList $args -WorkingDirectory $raizProjeto `
                -WindowStyle Hidden -PassThru -Wait -RedirectStandardOutput $stdout -RedirectStandardError $stderr
            $timer.Stop()
            if ($processo.ExitCode -ne 0) { throw "AGM falhou N=$n repeticao=$rep : $(Get-Content -Raw $stderr)" }
            $saidaTexto = Get-Content -Raw -LiteralPath $stdout
            $v = [regex]::Match($saidaTexto, '(?m)^Vertices: (\d+)$')
            $e = [regex]::Match($saidaTexto, '(?m)^Arestas selecionadas: (\d+)$')
            $custo = [regex]::Match($saidaTexto, '(?m)^Fibra minima: ([0-9.]+) km$')
            $mem = [regex]::Match($saidaTexto, '(?m)^Memoria estimada \(' + $Estrutura + '\): (\d+) bytes \(modelo de alocacoes\)$')
            if (-not ($v.Success -and $e.Success -and $custo.Success -and $mem.Success) -or [int]$v.Groups[1].Value -ne $n) {
                throw "Saida AGM ou instrumentacao incompleta em N=$n repeticao=$rep. Consulte $stdout"
            }
            $registros.Add([pscustomobject]@{
                benchmark_id = $idExecucao; estrutura = $Estrutura; dataset = $dataset
                repeticao = $rep; vertices = [int]$v.Groups[1].Value
                arestas = [int]$e.Groups[1].Value
                tempo_parede_total_ms = [string]::Format($cult, '{0:F3}', $timer.Elapsed.TotalMilliseconds)
                memoria_estimada_bytes = [long]$mem.Groups[1].Value
                custo_agm_km = [string]::Format($cult, '{0:F4}', [double]::Parse($custo.Groups[1].Value, $cult))
                compilador = $compilador; sistema = [Environment]::OSVersion.ToString()
                processador = $processador
            })
            Write-Output "$Estrutura N=$n repeticao=${rep}: V=$($v.Groups[1].Value), E=$($e.Groups[1].Value), custo=$($custo.Groups[1].Value) km, memoria=$($mem.Groups[1].Value) bytes, parede=$($timer.Elapsed.TotalMilliseconds.ToString('F3', $cult)) ms"
        }
    }
    $registros | Export-Csv -LiteralPath $destino -NoTypeInformation -Encoding UTF8
    Write-Output "Benchmark salvo em $destino ($($registros.Count) execucoes)."
} finally { Pop-Location }
