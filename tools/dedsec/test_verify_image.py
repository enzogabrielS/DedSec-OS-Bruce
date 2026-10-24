"""Host tests of flash-safety checks. No hardware or serial communication."""
import shutil
import tempfile
import unittest
from pathlib import Path

from verify_image import verify


class ImageSafetyTests(unittest.TestCase):
    def setUp(self):
        self.workspace = tempfile.TemporaryDirectory()
        self.addCleanup(self.workspace.cleanup)
        self.root = Path(self.workspace.name)
        self.environment = "CYD-test"
        shutil.copyfile(Path(__file__).resolve().parents[2] / "custom_4Mb_full.csv",
                        self.root / "custom_4Mb_full.csv")
        self.build = self.root / ".pio" / "build" / self.environment
        self.build.mkdir(parents=True)
        self.app = self.build / "firmware.bin"
        self.merged = self.root / f"Bruce-{self.environment}.bin"
        data = bytes([0xE9]) + bytes(range(255))
        self.app.write_bytes(data)
        combined = bytearray([0xFF]) * (0x10000 + len(data))
        combined[0x10000:] = data
        for name, offset, content in [("bootloader.bin", 0x1000, b"boot-test"),
                                      ("partitions.bin", 0x8000, b"table-test")]:
            (self.build / name).write_bytes(content)
            combined[offset:offset + len(content)] = content
        self.merged.write_bytes(combined)

    def test_complete_image_with_correct_offsets(self):
        result = verify(self.root, self.environment)
        self.assertEqual(result["application_bytes"], 256)
        self.assertEqual(result["application_partition_bytes"], 0x3C0000)

    def test_reject_application_beyond_factory(self):
        self.app.write_bytes(b"\xe9" + bytes(0x3C0000))
        with self.assertRaisesRegex(ValueError, "factory"):
            verify(self.root, self.environment)

    def test_reject_application_at_wrong_address(self):
        with self.merged.open("r+b") as handle:
            handle.seek(0x10000)
            handle.write(b"\x00")
        with self.assertRaisesRegex(ValueError, "application offset"):
            verify(self.root, self.environment)

    def test_reject_missing_bootloader(self):
        with self.merged.open("r+b") as handle:
            handle.seek(0x1000)
            handle.write(b"\xff")
        with self.assertRaisesRegex(ValueError, "bootloader"):
            verify(self.root, self.environment)

    def test_reject_image_reaching_littlefs(self):
        with self.merged.open("ab") as handle:
            handle.truncate(0x3D0001)
        with self.assertRaisesRegex(ValueError, "filesystem"):
            verify(self.root, self.environment)

    def test_reject_non_esp_application(self):
        self.app.write_bytes(b"not an ESP32 image")
        with self.assertRaisesRegex(ValueError, "magic"):
            verify(self.root, self.environment)


if __name__ == "__main__":
    unittest.main()
