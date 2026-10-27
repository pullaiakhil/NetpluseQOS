$packages = Get-ChildItem "$env:LOCALAPPDATA\Microsoft\WinGet\Packages" -Recurse -ErrorAction SilentlyContinue
Write-Output "Found package files: $($packages.Count)"
$gcc = Get-ChildItem "$env:LOCALAPPDATA\Microsoft\WinGet\Packages" -Recurse -Filter "gcc.exe" -ErrorAction SilentlyContinue
if ($gcc) {
    Write-Output "GCC found at: $($gcc.FullName)"
}
$linkDirs = Get-ChildItem "$env:LOCALAPPDATA\Microsoft\WinGet\Links" -ErrorAction SilentlyContinue
if ($linkDirs) {
    Write-Output "WinGet Links:"
    $linkDirs | Select-Object -ExpandProperty FullName
}
