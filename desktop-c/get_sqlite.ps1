$destZip = "c:\Users\AKHIL PULLAI\Downloads\cnn\desktop-c\third_party\sqlite\sqlite.zip"
$destDir = "c:\Users\AKHIL PULLAI\Downloads\cnn\desktop-c\third_party\sqlite"
$url = "https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip"
Write-Output "Downloading SQLite amalgamation from $url..."
try {
    Invoke-WebRequest -Uri $url -OutFile $destZip -UseBasicParsing
    Write-Output "Extracting SQLite..."
    Expand-Archive -Path $destZip -DestinationPath $destDir -Force
    $extractedFolder = Get-ChildItem -Path $destDir -Directory | Where-Object { $_.Name -like "sqlite-amalgamation*" }
    if ($extractedFolder) {
        Copy-Item "$($extractedFolder.FullName)\sqlite3.c" "$destDir\" -Force
        Copy-Item "$($extractedFolder.FullName)\sqlite3.h" "$destDir\" -Force
        Copy-Item "$($extractedFolder.FullName)\sqlite3ext.h" "$destDir\" -Force
        Write-Output "SQLite 3 amalgamated C source installed successfully!"
    }
} catch {
    Write-Output "Error downloading SQLite: $_"
}
