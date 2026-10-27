Write-Output "--- winget ---"
C:\Windows\System32\where.exe winget 2>$null
Write-Output "--- scoop or choco ---"
C:\Windows\System32\where.exe scoop choco 2>$null
Write-Output "--- Visual Studio folders ---"
Get-ChildItem "C:\Program Files\Microsoft Visual Studio" -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
Get-ChildItem "C:\Program Files (x86)\Microsoft Visual Studio" -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
Write-Output "--- Program Files general compilers ---"
Get-ChildItem "C:\Program Files" -ErrorAction SilentlyContinue | Where-Object { $_.Name -match "mingw|llvm|clang|gcc|cmake|visual" } | Select-Object -ExpandProperty FullName
Get-ChildItem "C:\Program Files (x86)" -ErrorAction SilentlyContinue | Where-Object { $_.Name -match "mingw|llvm|clang|gcc|cmake|visual" } | Select-Object -ExpandProperty FullName
