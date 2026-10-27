$env:PATH = "C:\mingw64\bin;C:\Windows\System32;C:\Windows;C:\Windows\System32\WindowsPowerShell\v1.0"
Set-Location "c:\Users\AKHIL PULLAI\Downloads\cnn\desktop-c\build"

Write-Output "=== Re-configuring CMake ==="
& cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER="C:/mingw64/bin/gcc.exe"

Write-Output "=== Compiling test_live_network ==="
& cmake --build . --target test_live_network

Write-Output "=== Executing test_live_network ==="
& .\test_live_network.exe
