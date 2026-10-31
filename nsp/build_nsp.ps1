# Builds Shank 2 as an NSP (Carafe's approach): the ExeFS is a small loader (nsp\loader, based on nx-hbloader) that
# starts shank2.nro from the NSP's RomFS in application mode. Settings, saves and logs go to /switch/shank2-nx/ on the
# SD card. Packed with hacBrewPack and your own prod.keys (never part of this repository).
#
# Two kinds:
#   -Release   the NSP that can be shared: only the loader and the release NRO (port/out/nx-release, no translated game
#              shader inside). The game files and shaders come from the SD card (the installer page's zip:
#              /switch/shank2-nx/game_root.nxpack, .nxidx and deko3d_glsl.bin), which the game finds by itself when
#              its RomFS has no game pack (port/src/shank2_nsp.cpp).
#   (default)  a personal, self-contained NSP: the development NRO plus the game pack in the RomFS (game data: never
#              share it).
#
#   pwsh -File nsp/build_nsp.ps1 [-Release] [-Version 1.0.0] [-Keys <prod.keys>]
# PowerShell 7 only: Windows PowerShell 5 cannot run the -replace with a script block below ("loader build failed").
param(
  [switch]$Release,
  [string]$Version = '1.0.0',
  # The release NSP and the personal one have their own title IDs, so both can be installed side by side.
  [string]$TitleId = '',
  [string]$Title = 'Shank 2',
  [string]$Publisher = 'github.com/zhtipu',
  [string]$Keys = '',          # default: ~/.switch/prod.keys
  [string]$HacBrewPack = ''    # default: hacbrewpack on PATH
)
$ErrorActionPreference = 'Stop'
if (-not $TitleId) { $TitleId = if ($Release) { '0177110ea0dc0000' } else { '0177110ea0dd0000' } }
$root = Split-Path $PSScriptRoot -Parent
$nsp = $PSScriptRoot
$sd = "$root\port\out\sdcard\switch\shank2-nx"
$nro = if ($Release) { "$root\port\out\nx-release\shank2.nro" } else { "$root\port\out\nx\shank2.nro" }
$icon = "$root\icon.ico"

if (-not $HacBrewPack) {
  $HacBrewPack = 'C:\Program Files\Carafe\hacbrewpack.exe'
  if (-not (Test-Path $HacBrewPack)) { $HacBrewPack = (Get-Command hacbrewpack -ErrorAction Stop).Source }
}
if (-not $Keys) {
  $carafe = "$env:APPDATA\app.carafe.builder\preferences.json"
  $Keys = if (Test-Path $carafe) { (Get-Content $carafe -Raw | ConvertFrom-Json).keysPath } else { "$HOME\.switch\prod.keys" }
}
$need = @($HacBrewPack, $nro, $icon, $Keys)
if (-not $Release) { $need += @("$sd\game_root.nxpack", "$sd\game_root.nxidx") }
foreach ($f in $need) { if (-not (Test-Path -LiteralPath $f)) { throw "Missing: $f" } }

# 1. The loader (devkitPro's make cannot handle spaces in the path: build a copy elsewhere).
$env:PATH = ($env:PATH -replace '"', '')
$tmp = "$env:LOCALAPPDATA\Temp\shank2loader"
if (Test-Path $tmp) { Remove-Item $tmp -Recurse -Force }
Copy-Item "$nsp\loader" $tmp -Recurse -Exclude build
$unix = '/' + ($tmp -replace '\\', '/' -replace '^([A-Za-z]):', { $_.Groups[1].Value.ToLower() })
& 'C:\devkitPro\msys2\usr\bin\bash.exe' -lc "export DEVKITPRO=/opt/devkitpro DEVKITA64=/opt/devkitpro/devkitA64; cd '$unix' && make -j4 >/dev/null"
if ($LASTEXITCODE -ne 0) { throw 'loader build failed' }

# 2. Staging (hacBrewPack edits main.npdm and control.nacp in place, so they are copies).
$stage = "$nsp\stage"
if (Test-Path $stage) { Remove-Item $stage -Recurse -Force }
New-Item -ItemType Directory "$stage\exefs", "$stage\romfs", "$stage\control", "$stage\out" | Out-Null
Copy-Item "$tmp\build\shank2-loader.nso" "$stage\exefs\main"
Copy-Item "$tmp\build\shank2-loader.npdm" "$stage\exefs\main.npdm"
Copy-Item $nro "$stage\romfs\shank2.nro"
if (-not $Release) {
  # The 1.2 GB pack: a hard link when it is on the same drive (hacBrewPack only reads it).
  foreach ($name in 'game_root.nxpack', 'game_root.nxidx') {
    $dest = "$stage\romfs\$name"
    try { New-Item -ItemType HardLink -Path $dest -Target "$sd\$name" | Out-Null } catch { Copy-Item "$sd\$name" $dest }
  }
}

# 3. Icon: 256x256 baseline JPEG from the .ico; NACP from nacptool, then the IDs and language flag set directly.
python -I "$nsp\make_control.py" --icon $icon --out "$stage\control" --title $Title --publisher $Publisher `
  --version $Version --titleid $TitleId
if ($LASTEXITCODE -ne 0) { throw 'control data failed' }

# 4. Pack.
Push-Location $stage
& $HacBrewPack --keyset $Keys --titleid $TitleId --exefsdir exefs --romfsdir romfs --controldir control --nologo --nspdir out `
  --ncadir nca --tempdir temp --backupdir backup
$code = $LASTEXITCODE
Pop-Location
if ($code -ne 0) { throw "hacBrewPack failed ($code)" }
$built = Get-ChildItem "$stage\out" -Filter *.nsp | Select-Object -First 1
$final = if ($Release) { "$nsp\shank2-nx-$Version.nsp" } else { "$nsp\$Title [$($TitleId.ToUpper())][v0].nsp" }
Move-Item -LiteralPath $built.FullName -Destination $final -Force  # LiteralPath: the [ ] in the name are not wildcards
Remove-Item "$stage\romfs\game_root.nxpack", "$stage\romfs\game_root.nxidx" -Force -ErrorAction SilentlyContinue
"NSP: $final ($([math]::Round((Get-Item -LiteralPath $final).Length / 1MB)) MB)"
