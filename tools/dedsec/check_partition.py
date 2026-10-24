"""PlatformIO post-build check of the actual factory partition."""
import csv
from pathlib import Path
Import("env")


def check_factory_image(target, source, env):
    root = Path(env.subst("$PROJECT_DIR"))
    table = root / env.GetProjectOption("board_build.partitions")
    with table.open(newline="", encoding="utf-8") as handle:
        rows = [[cell.strip() for cell in row] for row in csv.reader(handle)
                if row and not row[0].lstrip().startswith("#")]
    factory = next(row for row in rows if row[1:3] == ["app", "factory"])
    capacity = int(factory[4], 0)
    image = Path(env.subst("$BUILD_DIR")) / "firmware.bin"
    used = image.stat().st_size
    print(f"[DedSec] factory: {used}/{capacity} bytes; free={capacity - used}")
    if used > capacity:
        raise RuntimeError("DedSec application exceeds CYD factory partition")


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", check_factory_image)
