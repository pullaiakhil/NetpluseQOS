$env:PATH = 'C:\Windows\System32;C:\Windows;C:\Windows\System32\WindowsPowerShell\v1.0;' + $env:PATH
Write-Output "--- Testing compilers ---"
$tools = @('gcc', 'clang', 'cl', 'cmake', 'ninja', 'mingw32-make', 'make')
foreach ($t in $tools) {
    $res = C:\Windows\System32\where.exe $t 2>$null
    if ($res) {
        Write-Output "Found $t at: $res"
    } else {
        Write-Output "Not found on PATH: $t"
    }
}

Write-Output "--- Checking Common Directories ---"
$paths = @(
    'C:\msys64',
    'C:\msys64\mingw64\bin',
    'C:\msys64\ucrt64\bin',
    'C:\MinGW',
    'C:\MinGW\bin',
    'C:\mingw64',
    'C:\mingw64\bin',
    'C:\Program Files\LLVM\bin',
    'C:\Program Files\Git\usr\bin',
    'C:\Program Files\Microsoft Visual Studio',
    'C:\Program Files (x86)\Microsoft Visual Studio'
)

foreach ($p in $paths) {
    if (Test-Path $p) {
        Write-Output "Directory exists: $p"
    }
}
