#pragma once
#include <new/game_config.h>

#ifdef LEVEL_INFO_UTILS_ENABLED
#include <new/bases/d_level_info.hpp>

extern bool disableMessageScaling;

const wchar_t *getWorldName(int world);
const wchar_t *getLevelName(int dispWorld, int dispLevel);

const wchar_t *getWorldNumber(int world);
const wchar_t *getLevelNumber(int levelNumIdx);

ulong getLevelNumberIdx(u8 dispWorld, u8 dispLevel, u8 worldSlot, u8 levelSlot, bool doNotUseAnchor);
const wchar_t *getCombinedLevelNumber(int levelNumIdx);

u32 getStartingHouseKind();
#endif