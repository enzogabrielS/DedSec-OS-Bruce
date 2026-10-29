#pragma once
#ifdef DEDSEC_GRID
#include <globals.h>

// Draws existing Bruce options; actions are still executed by loopOptions.
void dedsecGridDraw(std::vector<Option> &items, int selected, bool main, const char *title, bool first);
void dedsecGridStatus();
// Nonnegative: chosen option. Negative: no hit, previous page, next page, back.
int dedsecGridHit(int x, int y, int count, int selected, bool main);
int dedsecGridPage(int count, int selected, bool main, int direction);
#endif
