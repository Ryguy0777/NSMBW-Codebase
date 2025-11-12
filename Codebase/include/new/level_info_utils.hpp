#pragma once
#include <new/game_config.h>

#ifdef LEVEL_INFO_UTILS_ENABLED

extern bool disableMessageScaling;

const wchar_t *getWorldName(int world);
const wchar_t *getLevelName(int world, int level);

const wchar_t *getWorldNumber(int world);
const wchar_t *getLevelNumber(int level);

ulong getLevelNumberIdx(u8 world, u8 level);
const wchar_t *getCombinedLevelNumber(int levelNumIdx);

u32 getStartingHouseKind();
#endif