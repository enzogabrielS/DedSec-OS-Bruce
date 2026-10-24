#pragma once
#include <stdint.h>

// RGB565. No images, additional font tables or framebuffer required.
namespace DedSec {
constexpr uint16_t Background = 0x0000;
constexpr uint16_t Surface = 0x10A2;
constexpr uint16_t Divider = 0x3186;
constexpr uint16_t Text = 0xFFFF;
constexpr uint16_t Muted = 0xBDF7;
constexpr uint16_t Accent = 0xF906;
constexpr uint16_t Success = 0x6D6B;
constexpr unsigned RefreshMs = 1000;
constexpr unsigned MaxNetworks = 24;
}
