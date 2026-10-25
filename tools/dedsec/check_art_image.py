"""Prove that every approved pixel payload is present in the packaged firmware."""
import hashlib
import json
import re
from pathlib import Path

from verify_image import require


def verify_art(root: Path, image: bytes) -> dict:
    metadata = json.loads((root / "media/dedsec/assets.json").read_text(encoding="utf-8"))
    source = (root / "src/core/dedsec_art_data.inc").read_text(encoding="utf-8")
    blocks = re.findall(r"static const uint8_t (\w+)Pixels\[\] PROGMEM = \{(.*?)\};", source, re.S)
    pixels = {name: bytes.fromhex(" ".join(re.findall(r"0x([0-9A-Fa-f]{2})", body))) for name, body in blocks}
    expected = {"Logo", "Wifi", "Bluetooth", "Config", "Rf", "Nrf24", "Lora", "Fm",
                "Infrared", "Ethernet", "Gps", "Rfid", "Files", "Scripts", "Clock", "Others", "Wallpaper"}
    require(set(pixels) == expected, "Incomplete artwork set")
    require({item["name"] for item in metadata} == expected and len(metadata) == len(expected),
            "Artwork metadata mismatch")
    for item in metadata:
        data = pixels[item["name"]]
        width, height = item["width"], item["height"]
        wallpaper = item["name"] == "Wallpaper"
        if wallpaper:
            require((width, height, item["bits_per_pixel"], item.get("max_luminance")) == (320, 240, 4, 48),
                    "Unexpected wallpaper format")
        else:
            require(0 < width <= 160 and 0 < height <= 160 and item["bits_per_pixel"] == 2,
                    "Unexpected artwork dimensions")
        per_byte = 8 // item["bits_per_pixel"]
        require(len(data) == ((width + per_byte - 1) // per_byte) * height == item["flash_bytes"],
                "Invalid packed length")
        require(hashlib.sha256(data).hexdigest() == item["packed_sha256"], "Packed artwork hash mismatch")
        original = root / "media/dedsec" / item["source"]
        require(hashlib.sha256(original.read_bytes()).hexdigest() == item["source_sha256"], "Original artwork changed")
        require(data in image, f"Artwork missing from compiled image: {item['name']}")
    return {"compiled_artworks": len(metadata), "pixel_bytes": sum(map(len, pixels.values()))}


if __name__ == "__main__":
    root = Path(__file__).resolve().parents[2]
    image = (root / "webflasher/firmware/DedSec-OS-CYD-2432S028.bin").read_bytes()
    print(json.dumps(verify_art(root, image), indent=2))
