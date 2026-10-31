# Loads the VS 2022 x64 build environment into the current PowerShell session.
# The machine PATH has a stray quote (C:\Program Files\PowerShell\7") that breaks vcvars64.bat, so strip it first.
$env:PATH = ($env:PATH -replace '"', '')
$bat = Join-Path $env:TEMP 'vsenv.bat'
Set-Content $bat '@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
set' -Encoding ascii
foreach ($l in (& cmd.exe /c $bat)) {
    if ($l -match '^([^=]+)=(.*)$') { [Environment]::SetEnvironmentVariable($matches[1], $matches[2]) }
}
$env:PATH = 'C:\Program Files\CMake\bin;C:\Program Files\LLVM\bin;' + $env:PATH
