param()
# Run in an activated ESP-IDF 5.5.3 PowerShell terminal. No private paths.
$ErrorActionPreference = 'Stop'
$repoPath = Split-Path -Parent $PSScriptRoot
if (-not $env:IDF_PATH) { throw 'Open an ESP-IDF 5.5.3 terminal first (IDF_PATH is missing).' }
$idfScript = Join-Path $env:IDF_PATH 'tools/idf.py'
if (-not (Test-Path -LiteralPath $idfScript)) { throw 'IDF_PATH does not contain tools/idf.py.' }
$idfVersion = (& python $idfScript --version | Out-String)
if ($LASTEXITCODE -ne 0 -or $idfVersion -notmatch 'ESP-IDF v5\.5\.3(?:\b|-)') {
    throw 'This project requires an activated ESP-IDF 5.5.3 Python environment.'
}
Push-Location $repoPath
try {
    $buildRoot = Join-Path $repoPath 'build'
    $buildPath = Join-Path $buildRoot ('native-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
    $imagePath = Join-Path $buildPath 'FoloToy-AI-Passport-full.bin'
    $oldDefaults = $env:SDKCONFIG_DEFAULTS
    try {
        $env:SDKCONFIG_DEFAULTS = Join-Path $repoPath 'sdkconfig.defaults'
        & python $idfScript -B $buildPath "-DSDKCONFIG=$buildPath/sdkconfig" build merge-bin -o $imagePath
        if ($LASTEXITCODE -ne 0) { throw 'ESP-IDF build or merge failed; no final image was replaced.' }
    } finally {
        $env:SDKCONFIG_DEFAULTS = $oldDefaults
    }
    & python tools/verify_firmware.py $buildPath
    if ($LASTEXITCODE -ne 0) { throw 'Merged image verification failed.' }
    & python tools/archive_firmware.py create $buildPath --archive-root (Join-Path $buildRoot 'firmware')
    if ($LASTEXITCODE -ne 0) { throw 'Debug archive verification failed.' }
    $imageHash = (Get-FileHash -LiteralPath $imagePath -Algorithm SHA256).Hash.ToLower()
    & python tools/archive_firmware.py verify (Join-Path (Join-Path $buildRoot 'firmware') $imageHash)
    if ($LASTEXITCODE -ne 0) { throw 'Archived firmware verification failed.' }
    Copy-Item -LiteralPath $imagePath -Destination (Join-Path $buildRoot 'FoloToy-AI-Passport-full.bin')
    Write-Output "Build: PASS. SHA-256: $imageHash"
    Write-Output 'Host tests: NOT RUN by this Windows build helper. Device tests: NOT RUN. Nothing was flashed.'
    Write-Output "Intermediate build retained at: $buildPath (safe to remove after archiving)."
} finally {
    Pop-Location
}
