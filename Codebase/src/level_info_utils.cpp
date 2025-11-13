#include <kamek.h>
#include <new/game_config.h>

#ifdef LEVEL_INFO_UTILS_ENABLED

#include <constants/game_constants.h>
#include <game/bases/d_message.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <new/bases/d_level_info.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>

#define STAGE_INVALID 42

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
const wchar_t *getLevelName(int displayWorld, int displayLevel) {
    EGG::MsgRes *msgRes = dMessage_c::getMesRes();
    if (msgRes->getMsgEntry(BMG_CATEGORY_LEVEL_NAMES+displayWorld, displayLevel) != nullptr)
        return dMessage_c::getMsg(BMG_CATEGORY_LEVEL_NAMES+displayWorld, displayLevel);
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

ulong getLevelNumberIdx(u8 world, u8 level, bool doNotUseAnchor) {
    dInfo_c::m_instance->mDisplayCourseWorld = world + 1;
    dInfo_c::m_instance->mDisplayCourseNum = level + 1;
    switch (level) {
        case 20: // ghost house
            return 2;
        case 21:  // tower
        case 22:
            return 3;
        case 23: // castle
        case 24:
            if (world == 7)
                return 5;
            return 4;
        case 25: // toad houses
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
            if (dScWMap_c::IsCourseType(world, level, dScWMap_c::COURSE_TYPE_KINOKO_HOUSE_1UP)) {
                return 6; // 1-up house
            } else if (dScWMap_c::IsCourseType(world, level, dScWMap_c::COURSE_TYPE_KINOKO_HOUSE_STAR)) {
                return 7; // star house
            } else {
                return 8; // red house
            }
        case 32: // ambush
        case 33:
        case 34:
            return 9;
        case 35: // cannon
            return 0xA;
        case 36: // unused? airship
            return 0xB;
        case 37: // airship
            if (dWmLib::isKoopaShipAnchor() && !doNotUseAnchor)
                return 0xC;
            return 0xB;
        case 38: // start point
            if (dWmLib::isStartPointKinokoHouseStar()) {
                return 7; // star house
            } else if (dWmLib::isStartPointKinokoHouseRed) {
                return 8; // red house
            } else {
                return 6; // 1-up house
            }
        case 40: // peach's castle
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

extern "C" bool isHomeCourseClear(int world); // 800FCB30

u32 getStartingHouseKind() {
    if (!isHomeCourseClear(dScWMap_c::m_WorldNo)) {
        return 0;
    } else {
        return dWmLib::getStartPointKinokoHouseKindNum();
    }
}
#endif