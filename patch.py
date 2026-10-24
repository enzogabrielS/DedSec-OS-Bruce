import hashlib
from typing import TYPE_CHECKING, Any
import requests
import subprocess
import os
from pathlib import Path

if TYPE_CHECKING:
    Import: Any = None
    env: Any = {}

import glob
import gzip
from os import makedirs, remove, rename
from os.path import basename, dirname, exists, isfile, join

Import("env")  # type: ignore

FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-arduinoespressif32-libs")
board_mcu = env.BoardConfig()
mcu = board_mcu.get("build.mcu", "")
patchflag_path = join(FRAMEWORK_DIR,mcu, "lib", ".patched")

# patch file only if we didn't do it befored
original_file = Path(FRAMEWORK_DIR) / mcu / "lib" / "libnet80211.a"
# Repair the interrupted state left by older patch.py before trusting its marker.
if not original_file.exists() and original_file.with_suffix(".a.old").exists():
    import shutil
    shutil.copy2(original_file.with_suffix(".a.old"), original_file)
    if Path(patchflag_path).exists():
        Path(patchflag_path).unlink()

if not isfile(patchflag_path) and mcu != "esp32p4":
    riscv = mcu in ("esp32c5", "esp32c6")
    package = "toolchain-riscv32-esp" if riscv else "toolchain-xtensa-esp-elf"
    toolchain = env.PioPlatform().get_package_dir(package)
    if not toolchain and not riscv:
        toolchain = env.PioPlatform().get_package_dir("toolchain-xtensa-" + mcu)
    if not toolchain:
        raise RuntimeError("Cannot find installed toolchain for Bruce WiFi patch")
    executable = "riscv32-esp-elf-objcopy" if riscv else "xtensa-" + mcu + "-elf-objcopy"
    if os.name == "nt":
        executable += ".exe"
    objcopy = Path(toolchain) / "bin" / executable
    patched_file = original_file.with_suffix(".a.patched")
    # An argv list handles spaces safely and a failed command leaves the library intact.
    subprocess.run([str(objcopy), "--weaken-symbol=ieee80211_raw_frame_sanity_check",
                    str(original_file), str(patched_file)], check=True)
    if not patched_file.is_file() or not patched_file.stat().st_size:
        raise RuntimeError("Bruce WiFi patch produced no output")
    patched_file.replace(original_file)
    Path(patchflag_path).touch()


def hash_file(file_path):
    """Generate SHA-256 hash for a single file."""
    hasher = hashlib.sha256()
    with open(file_path, "rb") as f:
        # Read the file in chunks to avoid memory issues
        for chunk in iter(lambda: f.read(4096), b""):
            hasher.update(chunk)
    return hasher.hexdigest()


def hash_files(file_paths):
    """Generate a combined hash for multiple files."""
    combined_hash = hashlib.sha256()

    for file_path in file_paths:
        file_hash = hash_file(file_path)
        combined_hash.update(file_hash.encode("utf-8"))  # Update with the file's hash

    return combined_hash.hexdigest()


def save_checksum_file(hash_value, output_file):
    """Save the hash value to a specified output file."""
    with open(output_file, "w") as f:
        f.write(hash_value)


def load_checksum_file(input_file):
    """Load the hash value from a specified input file."""
    with open(input_file, "r") as f:
        return f.readline().strip()


def minify_css(c):
    minify_req = requests.post(
        "https://www.toptal.com/developers/cssminifier/api/raw",
        {"input": c.read().decode('utf-8')},
    )
    return c if minify_req is False else minify_req.text.encode('utf-8')


def minify_js(js):
    minify_req = requests.post(
        'https://www.toptal.com/developers/javascript-minifier/api/raw',
        {'input': js.read().decode('utf-8')},
        timeout=10
    )
    return js if minify_req is False else minify_req.text.encode('utf-8')


def minify_html(html):
    minify_req = requests.post(
        'https://www.toptal.com/developers/html-minifier/api/raw',
        {'input': html.read().decode('utf-8')},
        timeout=10
    )
    return html if minify_req is False else minify_req.text.encode('utf-8')


# gzip web files
def prepare_www_files():
    HEADER_FILE = join(env.get("PROJECT_DIR"), "include", "webFiles.h")
    filetypes_to_gzip = ["html", "css", "js"]
    data_src_dir = join(env.get("PROJECT_DIR"), "embedded_resources/web_interface")
    checksum_file = join(data_src_dir, "checksum.sha256")
    checksum = ""

    if not exists(data_src_dir):
        print(f'Error: Source directory "{data_src_dir}" does not exist!')
        return

    if exists(checksum_file):
        checksum = load_checksum_file(checksum_file)

    files_to_gzip = []
    for extension in filetypes_to_gzip:
        files_to_gzip.extend(glob.glob(join(data_src_dir, "*." + extension)))

    files_checksum = hash_files(files_to_gzip)
    if files_checksum == checksum:
        print("[GZIP & EMBED INTO HEADER] - Nothing to process.")
        return

    print(f"[GZIP & EMBED INTO HEADER] - Processing {len(files_to_gzip)} files.")

    makedirs(dirname(HEADER_FILE), exist_ok=True)

    with open(HEADER_FILE, "w") as header:
        header.write(
            "#ifndef WEB_FILES_H\n#define WEB_FILES_H\n\n#include <Arduino.h>\n\n"
        )
        header.write(
            "// THIS FILE IS AUTOGENERATED DO NOT MODIFY IT. MODIFY FILES IN /embedded_resources/web_interface\n\n"
        )

        for file in files_to_gzip:
            gz_file = file + ".gz"
            with open(file, "rb") as src, gzip.open(gz_file, "wb") as dst:
                ext = basename(file).rsplit(".", 1)[-1].lower()
                if ext == 'html':
                    minified = minify_html(src)
                elif ext == 'css':
                    minified = minify_css(src)
                elif ext == 'js':
                    minified = minify_js(src)
                else:
                    raise ValueError(f"Unsupported file type: {ext}")

                # # Output minified file
                # min_file = file + ".min"
                # with open(min_file, "wb") as minf:
                #     minf.write(minified)

                dst.write(minified)

            with open(gz_file, "rb") as gz:
                compressed_data = gz.read()
                var_name = basename(file).replace(".", "_")

                header.write(f"const uint8_t {var_name}[] PROGMEM = {{\n")

                # Write hex values, inserting a newline every 15 bytes
                for i in range(0, len(compressed_data), 15):
                    hex_chunk = ", ".join(
                        f"0x{byte:02X}" for byte in compressed_data[i : i + 15]
                    )
                    header.write(f"  {hex_chunk},\n")

                header.write("};\n\n")
                header.write(
                    f"const uint32_t {var_name}_size = {len(compressed_data)};\n\n"
                )

            remove(gz_file)  # Clean up temporary gzip file

        header.write("#endif // WEB_FILES_H\n")

    save_checksum_file(files_checksum, checksum_file)

    print(f"[DONE] Gzipped files embedded into {HEADER_FILE}")


prepare_www_files()
