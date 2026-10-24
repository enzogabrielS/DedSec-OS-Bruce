#pragma once
#ifdef DEDSEC_OS
#include <MenuItemInterface.h>

class DedSecMenu : public MenuItemInterface {
public:
    DedSecMenu() : MenuItemInterface("System") {}
    void optionsMenu() override;
    void drawIcon(float scale) override;
    bool hasTheme() override { return false; }
    const String &themePath() override { return bruceConfig.theme.paths.config; }
};
#endif
