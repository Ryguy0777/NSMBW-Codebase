#include <new/game_config.h>

#ifdef NEWER_PREGAME
#include <kamek.h>

#include <game/bases/d_pregame_lyt.hpp>
#include <game/bases/d_info.hpp>
#include <game/mLib/m_heap.hpp>
#include <lib/revolution/TPL.h>
#include <new/constants/message_list.h>
#include <lib/egg/core/eggMsgRes.h>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_wm_lib.hpp>

// Change layout name
const char *newerPregameName = "preGame_newer/preGame.arc";
kmBranchDefAsm(0x80b6c074, 0x80b6c078) {
    lis r4, newerPregameName@h
    ori r4, r4, newerPregameName@l
    lwz r4, 0(r4)
    blr
}

// Longer pregame screen
kmWrite32(0x8091F144, 0x380000B4);

// Fix pane visibility? not sure
kmWrite32(0x80b6c1cc, 0x48000238);
kmWrite32(0x80b6c404, 0x7F83E378);

// Dummy out now-removed panes
kmWritePointer(0x80B88658, "T_x_00");
kmWritePointer(0x80B8865C, "T_x_00");
kmWritePointer(0x80B88660, "T_x_00");
kmWritePointer(0x80B88664, "T_x_00");
kmWritePointer(0x80B88668, "T_x_00");
kmWritePointer(0x80B886BC, "T_x_00");

kmWritePointer(0x80B88850, "P_bat_00");
kmWritePointer(0x80B88854, "P_bat_00");
kmWritePointer(0x80B88858, "P_bat_00");
kmWritePointer(0x80B8885C, "P_bat_00");
kmWritePointer(0x80B88860, "P_bat_00");
kmWritePointer(0x80B88864, "P_bat_00");
kmWritePointer(0x80B88868, "P_bat_00");
kmWritePointer(0x80B8886C, "P_bat_00");
kmWritePointer(0x80B88870, "P_bat_00");
kmWritePointer(0x80B88874, "P_bat_00");
kmWritePointer(0x80B88878, "P_bat_00");
kmWritePointer(0x80B888D4, "P_bat_00");

ulong pregameGetLevelNumberID(u8 world, u8 level) {
    dInfo_c::m_instance->mDisplayCourseWorld = world + 1;
    dInfo_c::m_instance->mDisplayCourseNum = level + 1;
    switch (level) {
        case 20: // Ghost house
            return 2;
        case 21:  // Tower
        case 22:
            return 3;
        case 23: // Castle
        case 24:
            if (world == 7) {
                return 5;
            }
            return 4;
        case 25: // Toad houses
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
            if (dScWMap_c::IsCourseType(world, level, dScWMap_c::COURSE_TYPE_KINOKO_HOUSE_1UP)) {
                return 6; // 1-up house
            } else if (dScWMap_c::IsCourseType(world, level, dScWMap_c::COURSE_TYPE_KINOKO_HOUSE_STAR)) {
                return 7; // Star house
            } else {
                return 8; // Red house
            }
        case 32: // Ambush
        case 33:
        case 34:
            return 9;
        case 35: // Cannon
            return 0xA;
        case 36: // Treasure ship (unused)
            return 0xB;
        case 37: // Airship
            if (dWmLib::isKoopaShipAnchor())
                return 0xC;
            return 0xB;
        case 38: // Start point
            if (dWmLib::isStartPointKinokoHouseStar()) {
                return 7; // Star house
            } else if (dWmLib::isStartPointKinokoHouseRed) {
                return 8; // Red house
            } else {
                return 6; // 1-up house
            }
        case 40: // Peach's castle
            return 0xD;
        default: // Normal levels
            return 1;
    }
}

void *pregameTPLbuffer = nullptr;

kmBranchDefCpp(0x80B6BDD0, NULL, void, dPreGameLyt_c *this_) {
    LytTextBox_c
        *LevelNumShadow, *LevelNum,
        *LevelNameShadow, *LevelName;

    LevelNumShadow = (LytTextBox_c *)this_->mLayout.findTextBoxByName("LevelNumShadow");
    LevelNum = (LytTextBox_c *)this_->mLayout.findTextBoxByName("LevelNum");
    LevelNameShadow = (LytTextBox_c *)this_->mLayout.findTextBoxByName("LevelNameShadow");
    LevelName = (LytTextBox_c *)this_->mLayout.findTextBoxByName("LevelName");

    MsgRes_c *msgRes = dMessage_c::getMesRes();
    // Grab names based on world + level number
    ulong category = BMG_CATEGORY_LEVEL_NAMES + dInfo_c::m_startGameInfo.mWorld1+1;
    ulong id = dInfo_c::m_startGameInfo.mLevel1+1;
    
    if (msgRes->getMsgEntry(category, id) == nullptr) {
        // The bmg entry for the level name doesn't exist, go to the fallback
        category = BMG_CATEGORY_LEVEL_NAMES;
        id = 0;
    }

    LevelNameShadow->setMessage(msgRes, category, id, 0);
    LevelName->setMessage(msgRes, category, id, 0);

    ulong number = pregameGetLevelNumberID(dInfo_c::m_startGameInfo.mWorld1, dInfo_c::m_startGameInfo.mLevel1);

    LevelNumShadow->setMessage(msgRes, BMG_CATEGORY_LEVEL_NAMES, number, 0);
    LevelNum->setMessage(msgRes, BMG_CATEGORY_LEVEL_NAMES, number, 0);

    nw4r::lyt::Picture *LevelSample;
    LevelSample = this_->mLayout.findPictureByName("LevelSample");

    // Get preview name
    char tplName[64];
    sprintf(tplName, "/LevelSamples/%02d-%02d.tpl", dInfo_c::m_startGameInfo.mWorld1+1, dInfo_c::m_startGameInfo.mLevel1+1);
    int entrynum = DVDConvertPathToEntrynum(tplName);
    if (entrynum == -1) {
        return;
    }

    // Load the file
    DVDFileInfo dvdHandle;
    bool fileLoaded = DVDFastOpen(entrynum, &dvdHandle);
    if (!fileLoaded) {
        return;
    }

    // Allocate memory
    pregameTPLbuffer = EGG::Heap::alloc(dvdHandle.size, 0x20, mHeap::g_archiveHeap);
    if (pregameTPLbuffer == nullptr) {
        return;
    }
    
    // Read file
    s32 length = DVDReadPrio(&dvdHandle, pregameTPLbuffer, dvdHandle.size, 0, 2);
    if (length > 0) {
        LevelSample->GetMaterial()->GetTexMapAry()->ReplaceImage((TPLPalette *)pregameTPLbuffer, 0, nullptr);
    }

    // Unload file
    DVDClose(&dvdHandle);
}

// Remove the tpl from memory
kmBranchDefCpp(0x80b6c580, NULL, bool, dPreGameLyt_c *this_) {
    if (pregameTPLbuffer != nullptr) {
        EGG::Heap::free(pregameTPLbuffer, mHeap::g_archiveHeap);
        pregameTPLbuffer = nullptr;
    }
    return this_->mLayout.doDelete();
}

#endif