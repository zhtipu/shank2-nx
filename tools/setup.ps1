# Prepares a fresh clone for building: the SDK base (tools/sa2-nx: SDK fork, renderer and tools) at the pinned commit with
# this port's changes (patches/sa2-nx.patch), its thirdparty folder copied into port/, and Microsoft's DXC binaries.
# Safe to run again: each step skips what is already done.
#
#   pwsh -File tools/setup.ps1
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$sa2 = Join-Path $root 'tools/sa2-nx'

# 1. The SDK base at the commit the port was made with (.gitmodules).
git -C $root submodule update --init --recursive tools/sa2-nx
if ($LASTEXITCODE -ne 0) { throw 'submodule update failed' }

# 2. This port's changes to the SDK (XMA multi-stream music, Switch sockets, NSP paths) and plume's D3D12 debug layer.
$patch = Join-Path $root 'patches/sa2-nx.patch'
git -C $sa2 apply --check -R $patch 2>$null
if ($LASTEXITCODE -eq 0) {
  'SDK base: patch already applied'
} else {
  git -C $sa2 apply --whitespace=nowarn $patch
  if ($LASTEXITCODE -ne 0) { throw 'patches/sa2-nx.patch does not apply (is tools/sa2-nx at the pinned commit?)' }
  'SDK base: patch applied'
}

# 3. The renderer's dependencies (plume, XenosRecomp, uam, imgui) where port/CMakeLists.txt looks for them.
$tp = Join-Path $root 'port/thirdparty'
if (-not (Test-Path $tp)) {
  Copy-Item (Join-Path $sa2 'app/thirdparty') $tp -Recurse
  'port/thirdparty: copied'
}

# 4. DXC (HLSL to SPIR-V for the shader steps): Microsoft's release v1.9.2609.
$dxc = Join-Path $tp 'reblue-XenosRecomp/thirdparty/dxc-bin'
if (-not (Test-Path (Join-Path $dxc 'bin/x64/dxc.exe'))) {
  $zip = Join-Path $env:TEMP 'dxc_2026_09_29.zip'
  Invoke-WebRequest 'https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2609/dxc_2026_09_29.zip' -OutFile $zip
  $x = Join-Path $env:TEMP 'dxc_2026_09_29'
  if (Test-Path $x) { Remove-Item $x -Recurse -Force }
  Expand-Archive $zip $x
  foreach ($d in 'bin/x64', 'lib/x64') {
    New-Item -ItemType Directory -Force (Join-Path $dxc $d) | Out-Null
    Copy-Item (Join-Path $x "$d/*") (Join-Path $dxc $d) -Recurse -Force
  }
  'DXC: v1.9.2609 installed'
}
'Ready. Next: docs/building.md'
