Write-Output "--- VS 18 ---"
Get-ChildItem "C:\Program Files\Microsoft Visual Studio\18" -Recurse -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
Write-Output "--- VS 2022 ---"
Get-ChildItem "C:\Program Files (x86)\Microsoft Visual Studio\2022" -Recurse -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
Write-Output "--- Git usr bin gcc? ---"
Get-ChildItem "C:\Program Files\Git\usr\bin" -Filter "*gcc*" -ErrorAction SilentlyContinue
Get-ChildItem "C:\Program Files\Git\mingw64\bin" -Filter "*gcc*" -ErrorAction SilentlyContinue
