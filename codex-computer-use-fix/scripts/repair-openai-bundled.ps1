$ErrorActionPreference = "Stop"

Write-Host "== Codex openai-bundled marketplace repair =="

$package = Get-AppxPackage 'OpenAI.Codex'
if (-not $package) {
    throw "OpenAI.Codex Windows App was not found. Install or update Codex first."
}

$marketplacePath = Join-Path $package.InstallLocation 'app\resources\plugins\openai-bundled'
Write-Host "Codex version: $($package.Version)"
Write-Host "Install path: $($package.InstallLocation)"
Write-Host "Bundled marketplace path: $marketplacePath"

if (-not (Test-Path -LiteralPath $marketplacePath)) {
    throw "openai-bundled marketplace path does not exist. Update or reinstall Codex, then try again."
}

Write-Host ""
Write-Host "Current marketplaces:"
codex plugin marketplace list

Write-Host ""
Write-Host "Refreshing openai-bundled..."
codex plugin marketplace remove openai-bundled
codex plugin marketplace add $marketplacePath

Write-Host ""
Write-Host "Updated marketplaces:"
codex plugin marketplace list

Write-Host ""
Write-Host "Done. Restart Codex, then check /plugins and Settings > Computer Use."
