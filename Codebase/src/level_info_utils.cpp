#include <kamek.h>
#include <new/game_config.h>

#ifdef LEVEL_INFO_UTILS_ENABLED

#include <constants/game_constants.h>
#include <game/bases/d_message.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>

#define STAGE_INVALID 42

// Returns the name of a world (from BMG_CATEGORY_KP_WORLD_NAMES)
// This uses the messageID from WorldInfo, not the world number
const wchar_t *getKoopatlasWorldName(int idx) {
    EGG::MsgRes *msgRes = dMessage_c::getMesRes();
    if (msgRes->getMsgEntry(BMG_CATEGORY_KP_WORLD_NAMES, idx) != nullptr)
        return dMessage_c::getMsg(BMG_CATEGORY_KP_WORLD_NAMES, idx);
    else
        return dMessage_c::getMsg(BMG_CATEGORY_KP_WORLD_NAMES, 0);
}

// Returns the name of a world
// Use the internal world number instead of the in-game one
const wchar_t *getWorldName(int world) {
    EGG::MsgRes *msgRes = dMessage_c::getMesRes();
    if (msgRes->getMsgEntry(BMG_CATEGORY_WORLD_NAMES, world+1) != nullptr)
        return dMessage_c::getMsg(BMG_CATEGORY_WORLD_NAMES, world+1);
    else
        return dMessage_c::getMsg(BMG_CATEGORY_WORLD_NAMES, 0);
}

// Returns the name of a level, uses the display IDs
const wchar_t *getLevelName(int dispWorld, int dispLevel) {
    EGG::MsgRes *msgRes = dMessage_c::getMesRes();
    if (msgRes->getMsgEntry(BMG_CATEGORY_LEVEL_NAMES+dispWorld, dispLevel) != nullptr)
        return dMessage_c::getMsg(BMG_CATEGORY_LEVEL_NAMES+dispWorld, dispLevel);
    else
        return dMessage_c::getMsg(BMG_CATEGORY_LEVEL_NAMES, 0);
}

const wchar_t *getWorldNumber(int world) {
    EGG::MsgRes *msgRes = dMessage_c::getMesRes();
    if (msgRes->getMsgEntry(BMG_CATEGORY_WORLD_NUMBERS, world) != nullptr)
        return dMessage_c::getMsg(BMG_CATEGORY_WORLD_NUMBERS, world);
    else
        return dMessage_c::getMsg(BMG_CATEGORY_WORLD_NUMBERS, 0);
}

// Returns a level number or icon
const wchar_t *getLevelNumber(int levelNumIdx) {
    EGG::MsgRes *msgRes = dMessage_c::getMesRes();
    if (msgRes->getMsgEntry(BMG_CATEGORY_LEVEL_ICONS, levelNumIdx) != nullptr)
        return dMessage_c::getMsg(BMG_CATEGORY_LEVEL_ICONS, levelNumIdx);
    else
        return dMessage_c::getMsg(BMG_CATEGORY_LEVEL_ICONS, 0);
}

ulong getLevelNumberIdx(u8 dispWorld, u8 dispLevel, u8 worldSlot, u8 levelSlot, bool doNotUseAnchor) {
    dInfo_c::m_instance->mDisplayCourseWorld = dispWorld;
    dInfo_c::m_instance->mDisplayCourseNum = dispLevel;
    switch (dispLevel) {
        case 21: // ghost house
            return 2;
        case 22:  // tower
        case 23:
            return 3;
        case 24: // castle
        case 25:
            if (dispWorld == 8)
                return 5;
            return 4;
        case 26: // toad houses
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
            if (dScWMap_c::IsCourseType(worldSlot, levelSlot, dScWMap_c::COURSE_TYPE_KINOKO_HOUSE_1UP)) {
                return 6; // 1-up house
            } else if (dScWMap_c::IsCourseType(worldSlot, levelSlot, dScWMap_c::COURSE_TYPE_KINOKO_HOUSE_STAR)) {
                return 7; // star house
            } else {
                return 8; // red house
            }
        case 33: // ambush
        case 34:
        case 35:
            return 9;
        case 36: // cannon
            return 0xA;
        case 37: // unused? airship
            return 0xB;
        case 38: // airship
            if (dWmLib::isKoopaShipAnchor() && !doNotUseAnchor)
                return 0xC;
            return 0xB;
        case 39: // start point
            if (dWmLib::isStartPointKinokoHouseStar()) {
                return 7; // star house
            } else if (dWmLib::isStartPointKinokoHouseRed()) {
                return 8; // red house
            } else if (dWmLib::isStartPointKinokoHouse1up()){
                return 6; // 1-up house
            } else {
                return (dWmLib::getStartPointType()) ? 0xE : 0xF;
            }
        case 41: // peach's castle
            return 0xD;
        default: // normal levels
            return 1;
    }
}

// Returns a combined "World X-X" identifier, used by Newer layouts
const wchar_t *getCombinedLevelNumber(int levelNumIdx) {
    EGG::MsgRes *msgRes = dMessage_c::getMesRes();
    if (msgRes->getMsgEntry(BMG_CATEGORY_LEVEL_NAMES, levelNumIdx) != nullptr)
        return dMessage_c::getMsg(BMG_CATEGORY_LEVEL_NAMES, levelNumIdx);
    else
        return dMessage_c::getMsg(BMG_CATEGORY_LEVEL_NAMES, 0);
}

// Update the hardcoded level list
kmBranchDefCpp(0x800B4F90, NULL, int, int worldNum, int levelNum) {
    if ((u32)worldNum > 8) {
        return STAGE_INVALID;
    }
    if ((u32)levelNum < 12) {
        //does our world exist?
        dLevelInfo_c::entry_s *liWorld = dLevelInfo_c::m_instance.getEntryFromSlotID(worldNum, 38);
        if (liWorld) {
            //if so, grab the corresponding section
            dLevelInfo_c::section_s *section = dLevelInfo_c::m_instance.getSection(worldNum);

            //create list of empty slots
            int levelNumList[12];
            for (int i = 0; i < 12; i++) {
                levelNumList[i] = 42;
            }

            //fill the list with our level slots
            int index = 0;
            for (int i = 0; i < section->mLevelCount; i++) {
                dLevelInfo_c::entry_s *level = &section->mLevels[i];
                if (level->mFlag & dLevelInfo_c::FLAG_VALID_LEVEL) {
                    levelNumList[index] = level->mLevelSlot;
                    index++;
                }
            }
            //return the level number
            return levelNumList[levelNum];
        } else {
            return STAGE_INVALID;
        }
    }
    return STAGE_INVALID;
}

// MultiCourseSelect helpers
int getWorldForButton(int page, int button) {
    dLevelInfo_c::entry_s *liWorld = dLevelInfo_c::m_instance.getEntryFromSlotID(page, 38);
    if (liWorld) {
        dLevelInfo_c::section_s *section = dLevelInfo_c::m_instance.getSection(page);
        for (int i = 0; i < section->mLevelCount; i++) {
            dLevelInfo_c::entry_s *level = &section->mLevels[i];
            if (level->mLevelSlot == dGameCom::getCourseNum(page, button)) {
                return level->mWorldSlot;
            }
        }
    }
    return 10;
}

int getRecommendedWorld(bool getFreeMode, int button) {
    int secIndex = dLevelInfo_c::m_instance.sectionCount() - (getFreeMode + 1);
    dLevelInfo_c::section_s *section = dLevelInfo_c::m_instance.getSection(secIndex);
    if (section) {
        dLevelInfo_c::entry_s *level = &section->mLevels[button+2];
        return (section->mLevelCount <= button+2) ? 10 : level->mWorldSlot;
    } else {
        return 10; // Invalid world ID
    }
}

int getRecommendedLevel(bool getFreeMode, int button) {
    int secIndex = dLevelInfo_c::m_instance.sectionCount() - (getFreeMode + 1);
    dLevelInfo_c::section_s *section = dLevelInfo_c::m_instance.getSection(secIndex);
    if (section) {
        dLevelInfo_c::entry_s *level = &section->mLevels[button+2];
        return (section->mLevelCount <= button+2) ? 42 : level->mLevelSlot;
    } else {
        return 42; // Invalid course ID
    }
}
#endif