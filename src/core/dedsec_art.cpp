#ifdef DEDSEC_OS
#include "dedsec_art.h"
#include "display.h"
#include <pgmspace.h>

namespace {
struct PackedArt { const uint8_t *pixels; uint16_t width; uint16_t height; };
#include "dedsec_art_data.inc"

// A small viewport describes a centered cover crop; no framebuffer is stored.
struct WallpaperView { int width, height, left, top, sourceWidth, sourceHeight; };

WallpaperView wallpaperView() {
    WallpaperView view = {tft.width(), tft.height(), 0, 0, WallpaperArt.width, WallpaperArt.height};
    if (view.width <= 0 || view.height <= 0) return view;
    if (view.width * view.sourceHeight > view.height * view.sourceWidth) {
        view.sourceHeight = view.sourceWidth * view.height / view.width;
        view.top = (WallpaperArt.height - view.sourceHeight) / 2;
    } else {
        view.sourceWidth = view.sourceHeight * view.width / view.height;
        view.left = (WallpaperArt.width - view.sourceWidth) / 2;
    }
    return view;
}

uint16_t wallpaperPixel(const WallpaperView &view, int x, int y) {
    if (x < 0 || y < 0 || x >= view.width || y >= view.height) return bruceConfig.bgColor;
    const unsigned sx = view.left + x * view.sourceWidth / view.width;
    const unsigned sy = view.top + y * view.sourceHeight / view.height;
    const uint8_t packed = pgm_read_byte(WallpaperArt.pixels + sy * ((WallpaperArt.width + 1) / 2) + sx / 2);
    const unsigned level = (packed >> (sx % 2 ? 0 : 4)) & 15;
    const unsigned gray = level * 48 / 15;
    return ((gray >> 3) << 11) | ((gray >> 2) << 5) | (gray >> 3);
}

const PackedArt &getArt(DedSecArt art) {
    switch (art) {
        case DedSecArt::Wifi: return WifiArt;
        case DedSecArt::Bluetooth: return BluetoothArt;
        case DedSecArt::Config: return ConfigArt;
        case DedSecArt::Rf: return RfArt;
        case DedSecArt::Nrf24: return Nrf24Art;
        case DedSecArt::Lora: return LoraArt;
        case DedSecArt::Fm: return FmArt;
        case DedSecArt::Infrared: return InfraredArt;
        case DedSecArt::Ethernet: return EthernetArt;
        case DedSecArt::Gps: return GpsArt;
        case DedSecArt::Rfid: return RfidArt;
        case DedSecArt::Files: return FilesArt;
        case DedSecArt::Scripts: return ScriptsArt;
        case DedSecArt::Clock: return ClockArt;
        case DedSecArt::Others: return OthersArt;
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

bool dedsecMenuWallpaperEnabled() {
    return bruceConfig.dedsecWallpaper && bruceConfig.themePath.isEmpty();
}

void dedsecDrawMenuWallpaper() {
    const WallpaperView view = wallpaperView();
    if (view.width <= 0 || view.height <= 27) return;
    uint16_t line[320];
    const bool oldSwap = tft.getSwapBytes();
    tft.setSwapBytes(true);
    // Bound each transfer to one row, including when the display is rotated.
    for (int y = 27; y < view.height; ++y) {
        for (int left = 0; left < view.width; left += 320) {
            const int count = min(320, view.width - left);
            for (int x = 0; x < count; ++x) line[x] = wallpaperPixel(view, left + x, y);
            tft.pushImage(left, y, count, 1, line);
        }
    }
    tft.setSwapBytes(oldSwap);
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
    const bool overlay = art != DedSecArt::Logo && dedsecMenuWallpaperEnabled();
    const WallpaperView view = wallpaperView();
    const bool oldSwap = tft.getSwapBytes();
    tft.setSwapBytes(true);
    for (int y = 0; y < height; ++y) {
        const unsigned row = (unsigned(y) * source.height / height) * stride;
        for (int x = 0; x < width; ++x) {
            const unsigned sx = unsigned(x) * source.width / width;
            const uint8_t packed = pgm_read_byte(source.pixels + row + sx / 4);
            const unsigned level = (packed >> (6 - 2 * (sx % 4))) & 3;
            line[x] = overlay ? blend(foreground, wallpaperPixel(view, left + x, top + y), level)
                              : palette[level];
        }
        // Animation shifts two small bands, without another copy of the bitmap.
        const int shift = bandShift && ((y / 12) % 4 == 1) ? bandShift : 0;
        tft.pushImage(left + shift, top + y, width, 1, line);
    }
    tft.setSwapBytes(oldSwap);
}
#endif
