$appx = Get-AppxPackage *DesktopAppInstaller* -ErrorAction SilentlyContinue
if ($appx) {
    Write-Output "Winget appx found at: $($appx.InstallLocation)"
    $wingetExe = Join-Path $appx.InstallLocation "winget.exe"
    if (Test-Path $wingetExe) {
        Write-Output "winget.exe exists: $wingetExe"
    }
} else {
    Write-Output "No DesktopAppInstaller Appx found"
}

$localApp = "$env:LOCALAPPDATA\Microsoft\WindowsApps\winget.exe"
if (Test-Path $localApp) {
    Write-Output "winget in WindowsApps: $localApp"
}
