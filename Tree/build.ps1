# build.ps1
Write-Host "--- Checking Environment ---" -ForegroundColor Cyan
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1"

if (!(Get-Command cl -ErrorAction SilentlyContinue)) {
    Write-Host "Error: cl.exe not found. Use 'Developer PowerShell for VS 2022'!" -ForegroundColor Red
    exit
}

# 1. Build CLI Tool
if (Test-Path "Tree\Tree.cpp") {
    Write-Host "Building CLI tool in Tree/..." -ForegroundColor Yellow
    Push-Location Tree
    cl /std:c++17 /EHsc /utf-8 Tree.cpp /out:mytree.exe
    Pop-Location
} else {
    Write-Host "Error: Tree\Tree.cpp not found!" -ForegroundColor Red
}

# 2. Build API Library
if (Test-Path "TreeAPI\Tree.cpp") {
    Write-Host "Building API DLL in TreeAPI/..." -ForegroundColor Yellow
    Push-Location TreeAPI
    cl /std:c++17 /EHsc /utf-8 /LD Tree.cpp /out:Tree.dll
    Pop-Location
} else {
    Write-Host "Error: TreeAPI\Tree.cpp not found!" -ForegroundColor Red
}

Write-Host "`n--- Finished ---" -ForegroundColor Green