"""Package the verified CYD build, or check the published package without PlatformIO."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

from verify_image import require, verify

ROOT = Path(__file__).resolve().parents[2]
FILENAME = "DedSec-OS-CYD-2432S028.bin"
REPORT = "docs/dedsec/DedSec-CYD-2432S028-image.json"


def check(root: Path) -> dict:
    site = root / "webflasher"
    manifest = json.loads((site / "manifest.json").read_text(encoding="utf-8"))
    release = json.loads((site / "release.json").read_text(encoding="utf-8"))
    report = json.loads((root / REPORT).read_text(encoding="utf-8"))
    require(manifest["builds"] == [{"chipFamily": "ESP32", "parts": [
        {"path": f"firmware/{FILENAME}", "offset": 0}]}], "Unexpected chip, file or offset")
    require(manifest["new_install_prompt_erase"] is True, "Erase must require a choice")
    require(manifest["new_install_improv_wait_time"] == 0, "Bruce does not provide Improv")
    require(release["version"] == manifest["version"], "Release/version mismatch")
    require(release["validation"] == report, "Release differs from validated build")
    data = (site / "firmware" / FILENAME).read_bytes()
    require(len(data) == report["merged_bytes"], "Merged image size mismatch")
    require(hashlib.sha256(data).hexdigest() == report["merged_sha256"], "Merged SHA-256 mismatch")
    require(len(data) <= 0x3D0000, "Image overwrites LittleFS")
    for offset in (0x1000, 0x10000):
        require(data[offset] == 0xE9, "ESP image missing")
        require(int.from_bytes(data[offset + 12:offset + 14], "little") == 0, "Not classic ESP32")
    require(data[0x1002:0x1004] == bytes([2, 0x20]), "Expected DIO / 4 MiB / 40 MHz")
    require(data[0x8000:0x8002] == b"\xAA\x50", "Partition table missing")
    app = data[0x10000:]
    require(len(app) == report["application_bytes"], "Application length mismatch")
    require(hashlib.sha256(app).hexdigest() == report["application_sha256"], "Application SHA-256 mismatch")
    factory = None
    for offset in range(0x8000, 0x8C00, 32):
        if data[offset:offset + 2] != b"\xAA\x50":
            break
        _, kind, subtype, start, size, _, _ = struct.unpack_from("<HBBII16sI", data, offset)
        require(start + size <= 0x400000, "Partition exceeds 4 MiB")
        if kind == 0 and subtype == 0:
            factory = (start, size)
    require(factory == (0x10000, 0x3C0000), "Unexpected factory partition")
    for filename in ("index.html", "styles.css", "app.mjs", "integrity.mjs", ".nojekyll"):
        require((site / filename).is_file(), f"Missing {filename}")
    return {"version": release["version"], "bytes": len(data), "sha256": report["merged_sha256"]}


def prepare(root: Path, version: str) -> None:
    result = verify(root, "DedSec-CYD-2432S028")
    report = json.loads((root / REPORT).read_text(encoding="utf-8"))
    require(result == report, "Build differs from checked-in validation report; review it first")
    site = root / "webflasher"
    (site / "firmware").mkdir(parents=True, exist_ok=True)
    (site / "firmware" / FILENAME).write_bytes((root / "Bruce-DedSec-CYD-2432S028.bin").read_bytes())
    manifest = {"name": "DedSec OS — CYD ESP32-2432S028", "version": version,
                "new_install_prompt_erase": True, "new_install_improv_wait_time": 0,
                "builds": [{"chipFamily": "ESP32", "parts": [{"path": f"firmware/{FILENAME}", "offset": 0}]}]}
    release = {"version": version, "board": "CYD ESP32-2432S028 padrão / ILI9341 resistiva / 4 MiB",
               "validation": result}
    for filename, value in (("manifest.json", manifest), ("release.json", release)):
        (site / filename).write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Check committed package; no build tools needed")
    parser.add_argument("--version", help="Explicit release identifier for a newly packaged build")
    args = parser.parse_args()
    if not args.check:
        if not args.version:
            parser.error("use --check, or --version to prepare")
        prepare(ROOT, args.version)
    print(json.dumps(check(ROOT), indent=2))
