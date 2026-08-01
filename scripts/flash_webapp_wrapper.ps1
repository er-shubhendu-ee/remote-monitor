<#*
 * @file      flash_webapp_wrapper.ps1
 * @author:   Shubhendu B B
 * @date:     02/08/2026
 * @brief     
 * @details   Distributed globally for free under the MIT License terms.
 * 
 * @copyright Copyright (c) 2025 er-shubhendu-ee
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *#>
# flash_webapp_wrapper.ps1
# ----------------------------------------------------------
# Load ESP-IDF environment and execute flash_webapp.py
# ----------------------------------------------------------

$ErrorActionPreference = "Stop"

# ----------------------------------------------------------
# Locate this script
# ----------------------------------------------------------

$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ProjectDir

# ----------------------------------------------------------
# Locate ESP-IDF export script
# ----------------------------------------------------------

$ExportScript = Join-Path `
    $env:USERPROFILE `
    ".vscode\extensions\espressif.esp-idf-extension-2.1.0\export.ps1"

if (!(Test-Path $ExportScript)) {
    Write-Host "[!] Cannot find:"
    Write-Host "    $ExportScript"
    exit 1
}

# ----------------------------------------------------------
# Load ESP-IDF environment
# ----------------------------------------------------------

Write-Host "[*] Loading ESP-IDF environment..."
. $ExportScript
Write-Host ""

# ----------------------------------------------------------
# Resolve paths
# ----------------------------------------------------------

$IDFPath = $env:IDF_PATH

if (-not $IDFPath) {
    Write-Host "[!] IDF_PATH not exported."
    exit 1
}

$InstallRoot = Split-Path $IDFPath -Parent
$Version     = Split-Path $InstallRoot -Leaf

$ToolsRoot = Join-Path $InstallRoot ".espressif"

$PythonExe = Join-Path `
    "C:\Espressif\tools\python" `
    "$Version\venv\Scripts\python.exe"

Write-Host "Resolved Paths"
Write-Host "--------------"
Write-Host "IDF_PATH        : $IDFPath"
Write-Host "Install Root    : $InstallRoot"
Write-Host "Version         : $Version"
Write-Host "IDF_TOOLS_PATH  : $ToolsRoot"
Write-Host "Python          : $PythonExe"
Write-Host ""

# ----------------------------------------------------------
# Verify
# ----------------------------------------------------------

if (!(Test-Path $PythonExe)) {
    Write-Host "[!] ESP-IDF Python not found:"
    Write-Host "    $PythonExe"
    exit 1
}

$env:IDF_PATH = $IDFPath
$env:IDF_TOOLS_PATH = $ToolsRoot
$env:IDF_PYTHON_ENV_PATH = Split-Path (Split-Path $PythonExe -Parent) -Parent

Write-Host "Compatibility Environment"
Write-Host "-------------------------"
Write-Host "IDF_PATH            = $env:IDF_PATH"
Write-Host "IDF_TOOLS_PATH      = $env:IDF_TOOLS_PATH"
Write-Host "IDF_PYTHON_ENV_PATH = $env:IDF_PYTHON_ENV_PATH"
Write-Host ""

& $PythonExe --version
if ($LASTEXITCODE -ne 0) {
    Write-Host "[!] Failed to execute ESP-IDF Python."
    exit $LASTEXITCODE
}

Write-Host ""

# ----------------------------------------------------------
# Execute flash script
# ----------------------------------------------------------

Write-Host "[*] Running flash_webapp.py"
Write-Host ""

& $PythonExe ".\flash_webapp.py"

$ExitCode = $LASTEXITCODE

Write-Host ""
Write-Host "[*] Finished (Exit Code = $ExitCode)"

exit $ExitCode