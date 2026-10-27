$env:PATH = "C:\mingw64\bin;C:\Windows\System32;C:\Windows;C:\Windows\System32\WindowsPowerShell\v1.0"
Set-Location "c:\Users\AKHIL PULLAI\Downloads\cnn\desktop-c\build"

Write-Output "=== Compiling Tests ==="
& cmake --build . --target test_qos_metrics
& cmake --build . --target test_database_reports
& cmake --build . --target netpulse_app

Write-Output "=== Running test_qos_metrics ==="
& .\test_qos_metrics.exe

Write-Output "=== Running test_database_reports ==="
& .\test_database_reports.exe
