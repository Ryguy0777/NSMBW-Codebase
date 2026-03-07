#pragma once

// Common Koopatlas resuables, to avoid unnecessary includes

#define KP_HUD WM_TEST
#define WM_SHOP WM_TOGEZO
#define KP_STAR_COIN_MENU COLLECTION_COIN

#ifdef KP_MAP_REPORT
#define MapReport OSReport
#else
#define MapReport(...)
#endif

#ifdef KP_SPAMMY_DEBUG
#define SpammyReport OSReport
#else
#define SpammyReport(...)
#endif

int SearchForIndexOfPlayerID(int id);