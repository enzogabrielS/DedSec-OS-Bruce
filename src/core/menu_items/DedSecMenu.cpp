#ifdef DEDSEC_OS
#include "DedSecMenu.h"
#include "core/dedsec_palette.h"
#include "core/utils.h"
#include "modules/ble/ble_common.h"
#include "modules/others/dedsec.h"

void DedSecMenu::optionsMenu() {
    returnToMenu = false;
    int selected = 0;
    // Build once per visit, and reuse across tools. Local options survive tools
    // which rebuild Bruce's global options, without repeated menu allocations.
    std::vector<Option> items = {
        {"Live dashboard", dedsecDashboard},
        {"WiFi survey", dedsecWifiSurvey},
        {"BLE discovery", ble_scan},
        {"Save diagnostic", dedsecSaveDiagnostic},
        {"Hardware / expansion", dedsecHardwareInfo},
        {"DedSec settings", dedsecSettings},
        {"Main Menu", []() {}}
    };
    do {
        selected = loopOptions(items, MENU_TYPE_SUBMENU, "DEDSEC / SYSTEM", selected);
    } while (!returnToMenu && selected >= 0 && selected < 6);
}

void DedSecMenu::drawIcon(float scale) {
    clearIconArea();
    const int w = 44 * scale, h = 30 * scale;
    tft.drawRect(iconCenterX - w, iconCenterY - h, w * 2, h * 2, bruceConfig.secColor);
    tft.fillRect(iconCenterX - w, iconCenterY - h, 3, h * 2, bruceConfig.priColor);
    tft.setTextColor(DedSec::Text, bruceConfig.bgColor);
    tft.setTextSize(3);
    tft.drawCentreString(">_", iconCenterX, iconCenterY - 12, 1);
}
#endif
