#ifdef DEDSEC_OS
#include "dedsec_art.h"
#include "display.h"
#include <pgmspace.h>

namespace {
struct PackedArt { const uint8_t *pixels; uint16_t width; uint16_t height; };
#include "dedsec_art_data.inc"

const PackedArt &getArt(DedSecArt art) {
    switch (art) {
        case DedSecArt::Wifi: return WifiArt;
        case DedSecArt::Bluetooth: return BluetoothArt;
        case DedSecArt::Config: return ConfigArt;
        default: return LogoArt;
    }
}

uint16_t blend(uint16_t foreground, uint16_t background, unsigned level) {
    const unsigned red = (((foreground >> 11) & 31) * level + ((background >> 11) & 31) * (3 - level)) / 3;
    const unsigned green = (((foreground >> 5) & 63) * level + ((background >> 5) & 63) * (3 - level)) / 3;
    const unsigned blue = ((foreground & 31) * level + (background & 31) * (3 - level)) / 3;
    return (red << 11) | (green << 5) | blue;
}
}

void dedsecDrawArt(DedSecArt art, int centerX, int centerY, int maxWidth, int maxHeight,
                   uint16_t foreground, uint16_t background, int bandShift) {
    if (maxWidth <= 0 || maxHeight <= 0) return;
    const PackedArt &source = getArt(art);
    int width = min(min(int(source.width), maxWidth), 160);
    int height = width * source.height / source.width;
    if (height > maxHeight) {
        height = maxHeight;
        width = height * source.width / source.height;
    }
    if (width <= 0 || height <= 0) return;
    const int left = centerX - width / 2, top = centerY - height / 2;
    const unsigned stride = (source.width + 3) / 4;
    const uint16_t palette[] = {background, blend(foreground, background, 1),
                              blend(foreground, background, 2), foreground};
    uint16_t line[160];
    const bool oldSwap = tft.getSwapBytes();
    tft.setSwapBytes(true);
    for (int y = 0; y < height; ++y) {
        const unsigned row = (unsigned(y) * source.height / height) * stride;
        for (int x = 0; x < width; ++x) {
            const unsigned sx = unsigned(x) * source.width / width;
            const uint8_t packed = pgm_read_byte(source.pixels + row + sx / 4);
            line[x] = palette[(packed >> (6 - 2 * (sx % 4))) & 3];
        }
        // Animation shifts two small bands, without another copy of the bitmap.
        const int shift = bandShift && ((y / 12) % 4 == 1) ? bandShift : 0;
        tft.pushImage(left + shift, top + y, width, 1, line);
    }
    tft.setSwapBytes(oldSwap);
}
#endif
