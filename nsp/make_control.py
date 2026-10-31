"""Control data for the Shank 2 NSP: icon_AmericanEnglish.dat (256x256 baseline JPEG) and control.nacp.

The NACP comes from devkitPro's nacptool; then the fields it leaves unset are written directly (offsets as in Carafe's
docs/nsp.md): the Title ID in PresenceGroupId, SaveDataOwnerId and LocalCommunicationId[0], AddOnContentBaseId =
Title ID + 0x1000, and SupportedLanguageFlag with American English, so the HOME Menu shows the name and icon.
"""
import argparse
import io
import os
import struct
import subprocess

from PIL import Image

NACPTOOL = r"C:\devkitPro\tools\bin\nacptool.exe"


def make_icon(src, dest):
    ico = Image.open(src)
    sizes = getattr(ico, "info", {}).get("sizes") or [ico.size]
    largest = max(sizes, key=lambda s: s[0] * s[1])
    try:
        ico.size = largest  # ICO: pick the largest frame
    except Exception:
        pass
    img = ico.convert("RGBA")
    canvas = Image.new("RGB", img.size, (0, 0, 0))
    canvas.paste(img, mask=img.split()[3])
    canvas = canvas.resize((256, 256), Image.LANCZOS)
    buf = io.BytesIO()
    canvas.save(buf, "JPEG", quality=95, progressive=False, optimize=False, subsampling=0)
    with open(dest, "wb") as f:
        f.write(buf.getvalue())
    return largest, len(buf.getvalue())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--icon", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--title", required=True)
    ap.add_argument("--publisher", required=True)
    ap.add_argument("--version", required=True)
    ap.add_argument("--titleid", required=True)
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)

    size, nbytes = make_icon(a.icon, os.path.join(a.out, "icon_AmericanEnglish.dat"))
    print(f"icon: {size[0]}x{size[1]} source -> 256x256 JPEG, {nbytes} bytes")

    nacp = os.path.join(a.out, "control.nacp")
    subprocess.run([NACPTOOL, "--create", a.title, a.publisher, a.version, nacp, f"--titleid={a.titleid}"],
                   check=True)
    data = bytearray(open(nacp, "rb").read())
    tid = int(a.titleid, 16)
    struct.pack_into("<I", data, 0x302C, 1)            # SupportedLanguageFlag: American English
    struct.pack_into("<Q", data, 0x3038, tid)          # PresenceGroupId
    struct.pack_into("<Q", data, 0x3070, tid + 0x1000)  # AddOnContentBaseId
    struct.pack_into("<Q", data, 0x3078, tid)          # SaveDataOwnerId
    struct.pack_into("<Q", data, 0x30B0, tid)          # LocalCommunicationId[0]
    open(nacp, "wb").write(data)
    name = data[0:0x200].split(b"\0")[0].decode()
    pub = data[0x200:0x300].split(b"\0")[0].decode()
    ver = data[0x3060:0x3070].split(b"\0")[0].decode()
    print(f"nacp: '{name}' by '{pub}', version {ver}, title id {a.titleid}")


if __name__ == "__main__":
    main()
