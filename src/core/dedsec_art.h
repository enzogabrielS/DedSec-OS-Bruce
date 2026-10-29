#pragma once

#ifdef DEDSEC_OS
#include <stdint.h>

enum class DedSecArt : uint8_t {
    Logo, Wifi, Bluetooth, Config, Rf, Nrf24, Lora, Fm, Infrared, Ethernet, Gps, Rfid,
    Files, Scripts, Clock, Others
};

bool dedsecMenuWallpaperEnabled();
void dedsecDrawMenuWallpaper();

// Source pixels stay in flash; at most one 160-pixel RGB565 row is on the stack.
void dedsecDrawArt(DedSecArt art, int centerX, int centerY, int maxWidth, int maxHeight,
                   uint16_t foreground, uint16_t background, int bandShift = 0, bool solidBackground = false);
#endif
