"""Checks of embedded 2-bit pixel ordering and row boundaries; no display required."""
import unittest

from encode_art import pack, unpack


class PackedArtTests(unittest.TestCase):
    def test_known_bit_order(self):
        self.assertEqual(pack([0, 1, 2, 3], 4, 1), b"\x1b")
        self.assertEqual(unpack(b"\x1b", 4, 1), [0, 1, 2, 3])

    def test_rows_have_independent_padding(self):
        self.assertEqual(pack([3, 2, 1, 0, 1, 2], 3, 2), b"\xe4\x18")
        self.assertEqual(unpack(b"\xe4\x18", 3, 2), [3, 2, 1, 0, 1, 2])

    def test_all_sizes_round_trip(self):
        for width in (1, 3, 4, 5, 127, 128, 160):
            for height in (1, 3, 149):
                pixels = [(x + 2 * y) % 4 for y in range(height) for x in range(width)]
                self.assertEqual(unpack(pack(pixels, width, height), width, height), pixels)

    def test_invalid_inputs_are_rejected(self):
        for values, width, height in (([], 0, 1), ([0], 2, 1), ([4], 1, 1), ([-1], 1, 1)):
            with self.assertRaises(ValueError):
                pack(values, width, height)
        with self.assertRaises(ValueError):
            unpack(b"", 5, 2)


if __name__ == "__main__":
    unittest.main()
