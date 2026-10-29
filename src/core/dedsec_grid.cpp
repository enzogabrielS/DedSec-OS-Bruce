#ifdef DEDSEC_GRID
#include "dedsec_grid.h"
#include "dedsec_art.h"
#include "dedsec_palette.h"
#include "display.h"
#include "modules/others/dedsec.h"

namespace {
constexpr uint16_t Ink = 0x0000, Panel = 0x10A2, Line = 0x3186, Red = 0xF945, White = 0xFFFF;
constexpr int Top = 44, Gap = 4, Margin = 6, Footer = 40;
int previousIndex = -1, previousPage = -1;

struct Rect {
    int x, y, w, h;
    bool contains(int px, int py) const { return px >= x && py >= y && px < x + w && py < y + h; }
};
Rect row(int slot, bool main) {
    const int columns = main ? 2 : 1;
    const int width = (tft.width() - 2 * Margin - (columns - 1) * Gap) / columns;
    const int height = (tft.height() - Footer - Top - 3 * Gap) / 3;
    return {Margin + (slot % columns) * (width + Gap), Top + (slot / columns) * (height + Gap), width, height};
}

DedSecArt icon(const String &label) {
    if (label == "WiFi") return DedSecArt::Wifi;
    if (label == "BLE" || label == "Bluetooth") return DedSecArt::Bluetooth;
    if (label == "RF") return DedSecArt::Rf;
    if (label == "NRF24") return DedSecArt::Nrf24;
    if (label == "LoRa") return DedSecArt::Lora;
    if (label == "FM") return DedSecArt::Fm;
    if (label == "IR") return DedSecArt::Infrared;
    if (label == "Ethernet") return DedSecArt::Ethernet;
    if (label == "GPS") return DedSecArt::Gps;
    if (label == "RFID") return DedSecArt::Rfid;
    if (label == "Files") return DedSecArt::Files;
    if (label == "JS Interpreter") return DedSecArt::Scripts;
    if (label == "Clock") return DedSecArt::Clock;
    if (label == "Config") return DedSecArt::Config;
    return DedSecArt::Others;
}

void card(const Option &option, int slot, bool selected, bool main) {
    const Rect r = row(slot, main);
    tft.fillRect(r.x, r.y, r.w, r.h, Panel);
    tft.drawRect(r.x, r.y, r.w, r.h, selected ? Red : Line);
    if (selected) tft.fillRect(r.x, r.y, 3, r.h, Red);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(main ? 1 : 2);
    const uint16_t fg = option.enabled ? White : DedSec::Muted;
    tft.setTextColor(fg, Panel);
    if (main) {
        const int size = min(36, r.h - 8);
        dedsecDrawArt(icon(option.label), r.x + 24, r.y + r.h / 2, size, size, fg, Panel, 0, true);
        const int font = r.w >= 140 ? 2 : 1;
        const char *label = option.label == "JS Interpreter" ? "Scripts" :
            (option.label == "Files" ? "Arquivos" : (option.label == "Clock" ? "Relogio" : option.label.c_str()));
        char text[40];
        snprintf(text, sizeof(text), "%.*s", (r.w - 54) / (6 * font), label);
        tft.setTextSize(font);
        tft.drawString(text, r.x + 48, r.y + 7, 1);
        tft.setTextSize(1);
        tft.setTextColor(DedSec::Muted, Panel);
        tft.drawString(dedsecHardwareRequirement(option.label) ? "EXT / OPCIONAL" : "BRUCE / MODULO",
                       r.x + 48, r.y + r.h - 16, 1);
    } else {
        const int chars = max(1, (r.w - 30) / 12);
        char text[40];
        snprintf(text, sizeof(text), "%.*s", chars, option.label.c_str());
        tft.drawString(text, r.x + 12, r.y + (r.h - 16) / 2, 1);
        if (option.selected) {
            tft.setTextSize(1);
            tft.drawRightString("*", r.x + r.w - 7, r.y + 5, 1);
        }
    }
    tft.setTextSize(1);
}
}

void dedsecGridStatus() {
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(DedSec::Muted, Ink);
    tft.fillRect(tft.width() - 96, 29, 90, 12, Ink);
    char heap[20];
    snprintf(heap, sizeof(heap), "HEAP %luK", static_cast<unsigned long>(ESP.getFreeHeap() / 1024));
    tft.drawRightString(heap, tft.width() - Margin, 31, 1);
}

void dedsecGridDraw(std::vector<Option> &items, int selected, bool main, const char *title, bool first) {
    const int perPage = main ? 6 : 3;
    const int page = selected / perPage;
    const bool full = first || previousPage != page;
    if (full) {
        tft.fillScreen(Ink);
        if (main && dedsecMenuWallpaperEnabled()) dedsecDrawMenuWallpaper();
        tft.fillRect(0, 0, tft.width(), Top, Ink);
        drawStatusBar();
        tft.setTextSize(1);
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(White, Ink);
        // Header stays clear of the real heap reading on the right.
        char heading[40];
        snprintf(heading, sizeof(heading), "%.*s", max(1, (tft.width() - 110) / 6), main ? "MODULOS" : title);
        tft.drawString(heading, 8, 31, 1);
        const int y = tft.height() - Footer;
        tft.fillRect(0, y, tft.width(), Footer, Ink);
        tft.drawFastHLine(0, y, tft.width(), Red);
        tft.setTextColor(White, Ink);
        tft.drawCentreString("< PAG", tft.width() / 6, y + 15, 1);
        tft.drawCentreString(main ? "ABRIR" : "VOLTAR", tft.width() / 2, y + 15, 1);
        tft.drawCentreString("PAG >", 5 * tft.width() / 6, y + 15, 1);
    }
    for (int slot = 0; slot < perPage; ++slot) {
        const int index = page * perPage + slot;
        if (index >= int(items.size())) {
            if (full) { const Rect r = row(slot, main); tft.fillRect(r.x, r.y, r.w, r.h, Ink); }
            continue;
        }
        if (full || index == selected || index == previousIndex) card(items[index], slot, index == selected, main);
    }
    // A thin page marker is separated from the 40-pixel touch controls.
    const int pages = (items.size() + perPage - 1) / perPage;
    if (full && pages > 0) {
        const int unit = max(2, (tft.width() - 12) / pages);
        tft.fillRect(6 + page * unit, tft.height() - Footer - 5, unit - 2, 2, Red);
    }
    dedsecGridStatus();
    previousIndex = selected;
    previousPage = page;
}

int dedsecGridHit(int x, int y, int count, int selected, bool main) {
    if (count <= 0 || x < 0 || x >= tft.width() || y < 0 || y >= tft.height()) return -1;
    if (!main && y < 27 && x < 40) return -4;
    if (y >= tft.height() - Footer) {
        if (x < tft.width() / 3) return -2;
        if (x >= 2 * tft.width() / 3) return -3;
        return main ? selected : -4;
    }
    const int perPage = main ? 6 : 3;
    const int start = selected / perPage * perPage;
    for (int slot = 0; slot < perPage; ++slot)
        if (start + slot < count && row(slot, main).contains(x, y)) return start + slot;
    return -1;
}

int dedsecGridPage(int count, int selected, bool main, int direction) {
    if (count <= 0) return -1;
    const int perPage = main ? 6 : 3;
    const int pages = (count + perPage - 1) / perPage;
    return ((selected / perPage + direction + pages) % pages) * perPage;
}
#endif
