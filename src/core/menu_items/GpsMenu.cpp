#include "GpsMenu.h"
#include "core/display.h"
#ifdef DEDSEC_OS
#include "core/dedsec_art.h"
#endif
#include "core/settings.h"
#include "core/utils.h"
#include "modules/gps/gps_tracker.h"
#include "modules/gps/wardriving.h"
#include <math.h>

void GpsMenu::optionsMenu() {
    options = {
        {"Wardriving",  [this]() { wardrivingMenu(); }},
#if !defined(LITE_VERSION)
        {"GPS Tracker", [=]() { GPSTracker(); }       },
#endif
        {"Config",      [this]() { configMenu(); }    },
    };
    addOptionToMainMenu();

    String txt = "GPS (" + String(bruceConfigPins.gpsBaudrate) + " bps)";
    loopOptions(options, MENU_TYPE_SUBMENU, txt.c_str());
}

void GpsMenu::wardrivingMenu() {
    options = {
        {"Scan WiFi Networks", []() { Wardriving(true, false); }},
        {"Scan BLE Devices",   []() { Wardriving(false, true); }},
        {"Scan Both",          []() { Wardriving(true, true); } },
        {"Back",               [this]() { optionsMenu(); }      },
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Wardriving");
}
void GpsMenu::configMenu() {
    options = {
        {"Baudrate", setGpsBaudrateMenu                                 },
        {"GPS Pins", [=]() { setUARTPinsMenu(bruceConfigPins.gps_bus); }},
        {"Back",     [this]() { optionsMenu(); }                        },
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "GPS Config");
}

void GpsMenu::drawIcon(float scale) {
    clearIconArea();
#ifdef DEDSEC_OS
    dedsecDrawArt(DedSecArt::Gps, iconCenterX, iconCenterY, iconAreaW - 8, iconAreaH - 8,
                  bruceConfig.priColor, bruceConfig.bgColor);
    return;
#endif
    int radius = scale * 18;
    if (radius % 2 != 0) radius++;

    int tangentX = sqrt(radius * radius - (radius / 2 * radius / 2));
    int32_t tangentY = radius / 2;

    tft.fillCircle(iconCenterX, iconCenterY - radius / 2, radius, bruceConfig.priColor);
    tft.fillTriangle(
        iconCenterX - tangentX,
        iconCenterY - radius / 2 + tangentY,
        iconCenterX + tangentX,
        iconCenterY - radius / 2 + tangentY,
        iconCenterX,
        iconCenterY + 1.5 * radius,
        bruceConfig.priColor
    );
    tft.fillCircle(iconCenterX, iconCenterY - radius / 2, radius / 2, bruceConfig.bgColor);

    tft.drawEllipse(iconCenterX, iconCenterY + 1.5 * radius, 1.5 * radius, radius / 2, bruceConfig.priColor);
}
