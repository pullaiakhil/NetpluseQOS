$env:PATH = "C:\mingw64\bin;C:\Windows\System32;C:\Windows;C:\Windows\System32\WindowsPowerShell\v1.0"
Set-Location "c:\Users\AKHIL PULLAI\Downloads\cnn\desktop-c"

if (Test-Path "build") {
    Remove-Item -Path "build" -Recurse -Force
}

Write-Output "Configuring CMake with C:\mingw64..."
& cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_C_COMPILER="C:/mingw64/bin/gcc.exe"

Write-Output "Building NetPulse..."
& cmake --build build --config Release
