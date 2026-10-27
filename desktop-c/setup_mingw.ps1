$src = "C:\Users\AKHIL PULLAI\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64"
$dst = "C:\mingw64"
if (-not (Test-Path $dst)) {
    Write-Output "Copying MinGW to C:\mingw64 to eliminate spaces in path..."
    Copy-Item -Path $src -Destination "C:\" -Recurse -Force
} else {
    Write-Output "C:\mingw64 already exists."
}
& "C:\mingw64\bin\gcc.exe" --version
