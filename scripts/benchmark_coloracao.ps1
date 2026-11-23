param(
    [string[]]$Estruturas = @('lista', 'matriz'),
    [string[]]$Algoritmos = @('gulosa', 'welsh-powell', 'exata'),
    [int[]]$Tamanhos = @(10, 20, 30, 40, 100, 500, 1000, 5000),
    [int]$Repeticoes = 5,
    [double]$LimiteSegundos = 60,
    [string]$Saida = ''
)

# Benchmark da coloracao de frequencias (issue #84): para cada subconjunto
# denso da Fase II (#70), estrutura e algoritmo, executa benchmark_coloracao.exe
# $Repeticoes vezes e grava uma linha por execucao. Execucoes da exata que
# estouram o limite de tempo ficam registradas com status limite_tempo e sem
# numero de cores.

$ErrorActionPreference = 'Stop'
if ($Repeticoes -lt 1) { throw 'Repeticoes deve ser maior que zero.' }
if ($LimiteSegundos -lt 0) { throw 'LimiteSegundos nao pode ser negativo.' }
$raizProjeto = Split-Path -Parent $PSScriptRoot
$executavel = Join-Path $raizProjeto 'build/benchmark_coloracao.exe'
$cult = [Globalization.CultureInfo]::InvariantCulture
$idExecucao = [guid]::NewGuid().ToString('N')
if ([string]::IsNullOrEmpty($Saida)) {
    $Saida = 'results/fase2/coloracao/coloracao_{0}.csv' -f (Get-Date -Format 'yyyyMMdd-HHmmss')
}
$destino = Join-Path $raizProjeto $Saida

function Obter-Campo([string]$texto, [string]$chave) {
    $m = [regex]::Match($texto, '(?m)^' + [regex]::Escape($chave) + ': ?(.*?)\r?$')
    if (-not $m.Success) { throw "Campo '$chave' ausente na saida do benchmark." }
    return $m.Groups[1].Value
}

Push-Location $raizProjeto
try {
    if (-not (Test-Path -LiteralPath $executavel)) {
        throw 'Execute mingw32-make benchmark-coloracao (ou compile build/benchmark_coloracao.exe) antes.'
    }
    if (Test-Path -LiteralPath $destino) { throw "O arquivo ja existe; escolha outro -Saida: $destino" }
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destino) | Out-Null
    $compilador = ((& gcc --version | Select-Object -First 1) -join '').Trim()
    $sistema = [Environment]::OSVersion.ToString()
    $processador = $env:PROCESSOR_IDENTIFIER
    $limiteTexto = $LimiteSegundos.ToString($cult)
    $registros = [System.Collections.Generic.List[object]]::new()

    foreach ($n in $Tamanhos) {
        $dataset = "data/subconjuntos/fase2/opencellid_denso_n$n.csv"
        if (-not (Test-Path -LiteralPath $dataset)) { throw "Subconjunto ausente: $dataset" }
        foreach ($estrutura in $Estruturas) {
            foreach ($algoritmo in $Algoritmos) {
                for ($rep = 1; $rep -le $Repeticoes; $rep++) {
                    $base = "build/coloracao-$idExecucao-$estrutura-$algoritmo-$n-$rep"
                    $stdout = Join-Path $raizProjeto "$base.stdout.log"
                    $stderr = Join-Path $raizProjeto "$base.stderr.log"
                    $argumentos = '--dataset "{0}" --estrutura {1} --algoritmo {2} --limite-segundos {3}' -f `
                        $dataset, $estrutura, $algoritmo, $limiteTexto
                    $processo = Start-Process -FilePath $executavel -ArgumentList $argumentos `
                        -WorkingDirectory $raizProjeto -WindowStyle Hidden -PassThru -Wait `
                        -RedirectStandardOutput $stdout -RedirectStandardError $stderr
                    $texto = Get-Content -Raw -LiteralPath $stdout
                    if ([string]::IsNullOrEmpty($texto)) {
                        throw "Sem saida em $algoritmo/$estrutura N=$n rep=${rep}: $(Get-Content -Raw $stderr)"
                    }
                    $status = Obter-Campo $texto 'Status'
                    if ($processo.ExitCode -ne 0 -or $status -eq 'erro') {
                        throw "Falha em $algoritmo/$estrutura N=$n rep=$rep. Consulte $stdout e $stderr"
                    }
                    $vertices = [int](Obter-Campo $texto 'Vertices')
                    if ($vertices -ne $n) { throw "Esperados $n vertices, lidos $vertices em $dataset." }
                    $exata = $algoritmo -eq 'exata'
                    $registro = [pscustomobject]@{
                        benchmark_id           = $idExecucao
                        algoritmo              = Obter-Campo $texto 'Algoritmo'
                        estrutura              = $estrutura
                        dataset                = $dataset
                        repeticao              = $rep
                        vertices               = $vertices
                        arestas                = [int](Obter-Campo $texto 'Arestas')
                        grau_maximo            = [int](Obter-Campo $texto 'Grau maximo')
                        tempo_ms               = Obter-Campo $texto 'Tempo ms'
                        memoria_grafo_bytes    = [long](Obter-Campo $texto 'Memoria grafo bytes')
                        memoria_auxiliar_bytes = [long](Obter-Campo $texto 'Memoria auxiliar bytes')
                        numero_cores           = Obter-Campo $texto 'Cores'
                        coloracao_valida       = Obter-Campo $texto 'Valida'
                        status                 = $status
                        limite_segundos        = if ($exata) { $limiteTexto } else { '' }
                        limite_inferior_exata  = if ($exata) { Obter-Campo $texto 'Limite inferior' } else { '' }
                        nos_visitados_exata    = if ($exata) { Obter-Campo $texto 'Nos visitados' } else { '' }
                        compilador             = $compilador
                        sistema                = $sistema
                        processador            = $processador
                    }
                    $registros.Add($registro)
                    Remove-Item -LiteralPath $stdout, $stderr -ErrorAction SilentlyContinue
                    Write-Output ("{0,-13} {1,-6} N={2,-5} rep={3}: E={4}, cores={5}, {6} ms, status={7}" -f `
                        $algoritmo, $estrutura, $n, $rep, $registro.arestas, $registro.numero_cores,
                        $registro.tempo_ms, $status)
                }
            }
        }
    }
    $registros | Export-Csv -LiteralPath $destino -NoTypeInformation -Encoding UTF8
    $interrompidas = @($registros | Where-Object { $_.status -ne 'ok' }).Count
    Write-Output "Benchmark salvo em $destino ($($registros.Count) execucoes, $interrompidas interrompidas)."
} finally { Pop-Location }
