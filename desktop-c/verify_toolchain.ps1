$mingwBin = "C:\Users\AKHIL PULLAI\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin"
$env:PATH = "$mingwBin;C:\Windows\System32;C:\Windows;C:\Windows\System32\WindowsPowerShell\v1.0;" + $env:PATH

Write-Output "--- Testing GCC ---"
& gcc --version
Write-Output "--- Testing CMake ---"
& cmake --version
Write-Output "--- Testing Make ---"
& mingw32-make --version
