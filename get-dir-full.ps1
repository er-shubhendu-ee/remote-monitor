# get-dir-full.ps1

$excludeDirs = 'Debug','Release','.settings','.metadata','build','Core','Drivers','Middlewares'
$excludeFiles = '.cproject','.project','dir-struct.txt','get-dir-full.ps1'

$root = Get-Location

Get-ChildItem -Recurse -Force |
Where-Object {

    $rel = $_.FullName.Substring($root.Path.Length + 1)

    # exclude directories anywhere in the path
    if ($rel.Split('\') | Where-Object { $excludeDirs -contains $_ }) { return $false }

    # exclude specific files
    if ($excludeFiles -contains $_.Name) { return $false }

    # exclude launch files
    if ($_.Name -like '*.launch') { return $false }

    return $true
} |
ForEach-Object {
    $_.FullName.Substring($root.Path.Length + 1)
} |
Out-File dir-struct.txt