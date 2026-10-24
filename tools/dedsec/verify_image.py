"""Fail closed if a CYD application/merged image exceeds the actual partition.

Uses only the Python standard library; does not flash or access serial ports.
"""
import argparse
import csv
import hashlib
import json
from pathlib import Path


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def verify(root: Path, environment: str) -> dict:
    with (root / "custom_4Mb_full.csv").open(newline="", encoding="utf-8") as handle:
        entries = [
            [column.strip() for column in row]
            for row in csv.reader(handle)
            if row and not row[0].lstrip().startswith("#")
        ]
    app = next(row for row in entries if row[1:3] == ["app", "factory"])
    offset, capacity = int(app[3], 0), int(app[4], 0)
    flash_capacity = 4 * 1024 * 1024
    for entry in entries:
        require(int(entry[3], 0) + int(entry[4], 0) <= flash_capacity, f"Partition outside flash: {entry}")
    binary = root / ".pio" / "build" / environment / "firmware.bin"
    merged = root / f"Bruce-{environment}.bin"
    boot = root / ".pio" / "build" / environment / "bootloader.bin"
    table = root / ".pio" / "build" / environment / "partitions.bin"
    image = binary.read_bytes()
    combined = merged.read_bytes()
    require(bool(image) and image[0] == 0xE9, "Missing ESP image magic")
    require(len(image) <= capacity, f"App {len(image)} > factory {capacity}")
    require(len(combined) <= offset + capacity, "Merged image reaches filesystem")
    require(combined[offset:offset + len(image)] == image, "Wrong application offset in merged image")
    for position, path in [(0x1000, boot), (0x8000, table)]:
        data = path.read_bytes()
        require(combined[position:position + len(data)] == data, f"Wrong {path.name} offset")
    return {
        "environment": environment,
        "application_bytes": len(image),
        "application_partition_bytes": capacity,
        "application_free_bytes": capacity - len(image),
        "merged_bytes": len(combined),
        "application_sha256": hashlib.sha256(image).hexdigest(),
        "merged_sha256": hashlib.sha256(combined).hexdigest(),
        "physical_tests": "not performed",
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--env", default="DedSec-CYD-2432S028")
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    result = verify(args.root, args.env)
    rendered = json.dumps(result, indent=2) + "\n"
    if args.output:
        args.output.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
