# Requires Administrator. Installs C++ workload into VS Build Tools 2026.
$ErrorActionPreference = "Stop"
$setup = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\setup.exe"
$path = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
$log = Join-Path $PSScriptRoot "msvc-install.log"

if (-not (Test-Path $setup)) {
    throw "VS Installer not found: $setup"
}

$args = @(
    "modify",
    "--installPath", $path,
    "--add", "Microsoft.VisualStudio.Workload.VCTools",
    "--add", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
    "--add", "Microsoft.VisualStudio.Component.Windows11SDK.22621",
    "--includeRecommended",
    "--quiet",
    "--wait",
    "--norestart"
)

Write-Host "Running: $setup $($args -join ' ')"
Write-Host "Log: $log"

$p = Start-Process -FilePath $setup -ArgumentList $args -Wait -PassThru -NoNewWindow
"ExitCode=$($p.ExitCode) Time=$(Get-Date -Format o)" | Tee-Object -FilePath $log
exit $p.ExitCode
