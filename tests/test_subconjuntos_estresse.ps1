$ErrorActionPreference = 'Stop'
$raizProjeto = Split-Path -Parent $PSScriptRoot
$executavel = Join-Path $raizProjeto 'build/grafo.exe'
$referencias = @(
    @{ n = 100; arestas = 2; sha256 = '25774DBA4E864B0207E85712A8AFBDE29613D2405F0A4508B05CB889390517C7' },
    @{ n = 500; arestas = 48; sha256 = '4CACFCA245705FA633A75AB468B2C7E7623317D271FA9E75DD93ACA0EEB84D3E' },
    @{ n = 1000; arestas = 186; sha256 = 'AF3657E93DEFB5F5B2E520F1FD53046D3672407BD0882F20ABDDFEE3D601FED8' },
    @{ n = 5000; arestas = 1897; sha256 = '591E73D967705BA75593EB9706EDDF75CE459B3D3191F7E99423673852CF8F79' }
)

Push-Location $raizProjeto
try {
    & python scripts/gerar_subconjuntos.py --check
    if ($LASTEXITCODE -ne 0) { throw 'Subconjuntos divergem do algoritmo de selecao versionado.' }
    foreach ($referencia in $referencias) {
        $n = $referencia.n
        $dataset = "data/subconjuntos/opencellid_n$n.csv"
        $csv = "build/subconjunto_n$n.csv"
        $hash = (Get-FileHash -LiteralPath $dataset -Algorithm SHA256).Hash
        if ($hash -ne $referencia.sha256) { throw "Hash inesperado para N=$n." }
        $saida = & $executavel --dataset $dataset --limite 0 --estrutura conjunta --saida $csv 2>&1
        if ($LASTEXITCODE -ne 0) { throw "Execucao falhou para N=$n : $($saida -join ' ')" }
        $texto = $saida -join "`n"
        if ($texto -notmatch "Vertices: $n\b" -or
            $texto -notmatch "Arestas: $($referencia.arestas)\b") {
            throw "Contagens divergentes no terminal para N=$n."
        }
        $registro = Import-Csv -LiteralPath $csv | Select-Object -Last 1
        if ($registro.vertices -ne "$n" -or $registro.arestas -ne "$($referencia.arestas)" -or
            $registro.registros_invalidos -ne '0' -or $registro.dataset -ne $dataset) {
            throw "Contagens divergentes no CSV para N=$n."
        }
        Write-Output "N=$n; V=$($registro.vertices); E=$($registro.arestas); SHA-256=$hash"
    }
    Write-Output 'Subconjuntos de estresse validados.'
} finally {
    Pop-Location
}
