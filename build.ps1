param([string]$Toolchain = "", [string]$Sdk = "")
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $arguments = @('build.py')
    if ($Toolchain) { $arguments += @('--toolchain', $Toolchain) }
    if ($Sdk) { $arguments += @('--sdk', $Sdk) }
    python @arguments
    if ($LASTEXITCODE -ne 0) { throw 'Yugioh 9588 build failed' }
} finally { Pop-Location }
