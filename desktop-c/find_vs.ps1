$vs1 = "C:\Program Files\Microsoft Visual Studio"
$vs2 = "C:\Program Files (x86)\Microsoft Visual Studio"
Get-ChildItem -Path $vs1 -Recurse -Filter "cl.exe" -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
Get-ChildItem -Path $vs2 -Recurse -Filter "cl.exe" -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

Get-ChildItem -Path $vs1 -Recurse -Filter "cmake.exe" -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
Get-ChildItem -Path $vs2 -Recurse -Filter "cmake.exe" -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
