#!/usr/bin/env python3
"""Dumps every Xbox 360 shader container Shank 2 draws with, for Renut's translator (sa2_renderer = "renut").

Shank 2 keeps all of its shaders in the effect files of data-360/data-360.zip (shaders/*.fxc), each holding whole
containers: nothing comes from default.xex. The zip is read in place (Python's zipfile), so nothing is extracted to
disk. Same container check and naming as sa2-nx's tools/sa2_dump_shaders.py (reNut's tools/dump_shaders.py): the
installer page does the same in the browser.

Output: <out>/{vs,ps}_<xxh3_64 of the container>.bin and <out>/manifest.csv.

  python tools/shank2_dump_shaders.py --game ../extracted --out out/renut_shader_dump
"""
import argparse
import csv
import os
import struct
import sys
import zipfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sa2_dump_shaders import find_shader_headers  # noqa: E402  (also checks that xxhash is installed)
import xxhash  # noqa: E402

ARCHIVE = os.path.join("data-360", "data-360.zip")


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--game", default=os.path.join(root, "..", "extracted"),
                    help="the game folder (default.xex, data/, data-360/)")
    ap.add_argument("--out", default=os.path.join(root, "out", "renut_shader_dump"))
    args = ap.parse_args()

    archive = os.path.join(args.game, ARCHIVE)
    if not os.path.isfile(archive):
        sys.exit("no %s" % archive)
    os.makedirs(args.out, exist_ok=True)
    seen = set()
    manifest = []
    with zipfile.ZipFile(archive) as z:
        for entry in sorted(n for n in z.namelist() if n.lower().endswith(".fxc")):
            data = z.read(entry)
            count = 0
            for h in find_shader_headers(data):
                flags, virt, phys = struct.unpack_from(">III", data, h)
                blob = data[h:h + virt + phys]
                kind = "vs" if flags & 1 else "ps"
                name = "%s_%s" % (kind, xxhash.xxh3_64_hexdigest(blob))
                manifest.append((name, kind, virt, phys, entry, "0x%x" % h))
                count += 1
                if name in seen:
                    continue
                seen.add(name)
                with open(os.path.join(args.out, name + ".bin"), "wb") as fh:
                    fh.write(blob)
            print("%-40s %4d shaders" % (entry, count))
    manifest.sort()
    with open(os.path.join(args.out, "manifest.csv"), "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["name", "type", "virtual_size", "physical_size", "source", "offset"])
        w.writerows(manifest)
    ps = sum(1 for n in seen if n.startswith("ps_"))
    print("%d unique shaders (%d ps, %d vs) -> %s" % (len(seen), ps, len(seen) - ps, args.out))


if __name__ == "__main__":
    main()
