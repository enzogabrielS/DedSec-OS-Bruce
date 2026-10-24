param(
    [ValidateSet('DedSec-CYD-2432S028', 'CYD-2432S028')]
    [string]$Environment = 'DedSec-CYD-2432S028'
)
$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$projectPython = Join-Path $projectRoot '.venv\Scripts\python.exe'
if (-not (Test-Path -LiteralPath $projectPython)) {
    throw 'Prepare .venv conforme docs/dedsec/README.md antes de compilar.'
}
$previousCore = $env:PLATFORMIO_CORE_DIR
$previousUtf8 = $env:PYTHONUTF8
Push-Location -LiteralPath $projectRoot
try {
    $env:PLATFORMIO_CORE_DIR = Join-Path $projectRoot '.pio-core'
    $env:PYTHONUTF8 = '1'
    & $projectPython -m platformio run -e $Environment
    if ($LASTEXITCODE -ne 0) { throw "Compilacao falhou: $LASTEXITCODE" }
    & $projectPython tools/dedsec/verify_image.py --env $Environment --output "docs/dedsec/$Environment-image.json"
    if ($LASTEXITCODE -ne 0) { throw 'Imagem rejeitada pela verificacao de particoes.' }
} finally {
    Pop-Location
    $env:PLATFORMIO_CORE_DIR = $previousCore
    $env:PYTHONUTF8 = $previousUtf8
}
