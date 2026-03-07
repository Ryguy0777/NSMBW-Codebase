#include <kamek.h>
#include <propelparts/game_config.h>

#ifdef LEVEL_INFO_ENABLED
#include <propelparts/bases/d_level_info.hpp>
#include <game/bases/d_dvd.hpp>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_save_mng.hpp>

dDvd::loader_c s_levelInfoLoader;
bool s_levelInfoLoaded = false;

dLevelInfo_c dLevelInfo_c::m_instance;

bool dLevelInfo_c::loadLevelInfo() {
    if (s_levelInfoLoaded) {
        return true;
    }

    void *data = s_levelInfoLoader.request("/LevelInfo.bin", 0, nullptr);
    if (data) {
        dLevelInfo_c::m_instance.init(data);
        s_levelInfoLoaded = true;
        return true;
    }

    return false;
}


void dLevelInfo_c::init(void *buffer) {
    mpData = (header_s*)buffer;

    // Decode all the level names
    for (int sect = 0; sect < sectionCount(); sect++) {
        // Parse this section
        section_s *thisSect = getSection(sect);

        for (int lev = 0; lev < thisSect->mLevelCount; lev++) {
            entry_s *level = &thisSect->mLevels[lev];

            if (level->mLevelSlot < STAGE_COUNT) {
                dScWMap_c::SetCourseType(level->mWorldSlot, level->mLevelSlot, (dScWMap_c::CourseType_e)level->mFlag);
            }

            char *name = (char*)getLevelName(level);
            for (int i = 0; i < level->mNameStrLen+1; i++) {
                name[i] -= 0xD0;
            }
        }
    }
}

dLevelInfo_c::entry_s *dLevelInfo_c::getEntryFromSlotID(int world, int level) {
    for (int i = 0; i < sectionCount(); i++) {
        section_s *sect = getSection(i);

        for (int j = 0; j < sect->mLevelCount; j++) {
            entry_s *entry = &sect->mLevels[j];
            if (entry->mWorldSlot == world && entry->mLevelSlot == level) {
                return entry;
            }
        }
    }

    return nullptr;
}

dLevelInfo_c::entry_s *dLevelInfo_c::getEntryFromDispID(int world, int level) {
    for (int i = 0; i < sectionCount(); i++) {
        section_s *sect = getSection(i);

        for (int j = 0; j < sect->mLevelCount; j++) {
            entry_s *entry = &sect->mLevels[j];
            if (entry->mDisplayWorld == world && entry->mDisplayLevel == level) {
                return entry;
            }
        }
    }

    return nullptr;
}

dLevelInfo_c::entry_s *dLevelInfo_c::getLastEntry(u32 sectionIdx) {
    section_s *sect = getSection(sectionIdx);

    entry_s *entry = &sect->mLevels[sect->mLevelCount-1];
    return entry;
}

// Update savefile star info
kmBranchDefCpp(0x801028D0, NULL, void) {
    dLevelInfo_c *li = &dLevelInfo_c::m_instance;
    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);

    bool coinsNormal = true;
    bool exitsNormal = true;
    bool coinsW9 = true;
    bool exitsW9 = true;

    for (int i = 0; i < li->sectionCount(); i++) {
        dLevelInfo_c::section_s *sect = li->getSection(i);

        for (int j = 0; j < sect->mLevelCount; j++) {
            dLevelInfo_c::entry_s *entry = &sect->mLevels[j];

            // Check actual levels only
            if (!(entry->mFlag & dLevelInfo_c::FLAG_VALID_LEVEL)) {
                continue;
            }

            u32 conds = save->getCourseDataFlag(entry->mWorldSlot, entry->mLevelSlot);

            if (entry->mDisplayWorld == LAST_SPECIAL_WORLD+1) {
                if ((conds & dMj2dGame_c::COIN_MASK) != dMj2dGame_c::COIN_MASK) {
                    coinsW9 = false;
                }

                if (entry->mFlag & dLevelInfo_c::FLAG_GOAL_NORMAL) {
                    if (!(conds & dMj2dGame_c::GOAL_NORMAL)) {
                        exitsW9 = false;
                    }
                }

                if (entry->mFlag & dLevelInfo_c::FLAG_GOAL_SECRET) {
                    if (!(conds & dMj2dGame_c::GOAL_SECRET)) {
                        exitsW9 = false;
                    }
                }
            } else {
                if ((conds & dMj2dGame_c::COIN_MASK) != dMj2dGame_c::COIN_MASK) {
                    coinsNormal = false;
                }

                if (entry->mFlag & dLevelInfo_c::FLAG_GOAL_NORMAL) {
                    if (!(conds & dMj2dGame_c::GOAL_NORMAL)) {
                        exitsNormal = false;
                    }
                }

                if (entry->mFlag & dLevelInfo_c::FLAG_GOAL_SECRET) {
                    if (!(conds & dMj2dGame_c::GOAL_SECRET)) {
                        exitsNormal = false;
                    }
                }
            }
        }
    }

    bool beatGame = (save->getCourseDataFlag(LAST_WORLD, STAGE_CASTLE) & dMj2dGame_c::GOAL_NORMAL) != 0;

    u8 bitfield = save->getGameCompletion();
    save->setGameCompletion(bitfield &= ~0x3C);
    save->setGameCompletion(bitfield |=
		(beatGame    ? dMj2dGame_c::FINAL_BOSS_BEATEN : 0) |
        (exitsNormal ? dMj2dGame_c::GOAL_ALL : 0) |
        (coinsNormal ? dMj2dGame_c::COIN_ALL : 0) |
        (exitsW9     ? dMj2dGame_c::COIN_ALL_SPECIAL : 0) |
        (coinsW9     ? dMj2dGame_c::GAME_COMPLETED : 0)
    );

    OSReport("SaveFile stars updated: Status: Game beaten: %d, Normal exits: %d, Normal coins: %d, Special exits: %d, Special coins: %d\n", beatGame, exitsNormal, coinsNormal, exitsW9, coinsW9);
}
#endif