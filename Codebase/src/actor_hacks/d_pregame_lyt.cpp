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

// longer pregame screen
kmWrite32(0x8091F144, 0x380000B4);

// fix pane visibility? not sure
kmWrite32(0x80b6c1cc, 0x48000238);
kmWrite32(0x80b6c404, 0x7F83E378);

// dummy out now-removed panes
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
            if (dWmLib::isKoopaShipAnchor())
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

kmBranchDefCpp(0x80B6BDD0, NULL, void, dPreGameLyt_c *_this) {
    LytTextBox_c
        *LevelNumShadow, *LevelNum,
        *LevelNameShadow, *LevelName;

    LevelNumShadow = (LytTextBox_c *)_this->mLayout.findTextBoxByName("LevelNumShadow");
    LevelNum = (LytTextBox_c *)_this->mLayout.findTextBoxByName("LevelNum");
    LevelNameShadow = (LytTextBox_c *)_this->mLayout.findTextBoxByName("LevelNameShadow");
    LevelName = (LytTextBox_c *)_this->mLayout.findTextBoxByName("LevelName");

    MsgRes_c *msgRes = dMessage_c::getMesRes();

    ulong category = BMG_CATEGORY_LEVEL_NAMES + dInfo_c::m_startGameInfo.mWorld1+1;
    ulong id = dInfo_c::m_startGameInfo.mLevel1+1;
    
    if (msgRes->getMsgEntry(category, id) == nullptr) {
        category = BMG_CATEGORY_LEVEL_NAMES;
        id = 0;
    }

    LevelNameShadow->setMessage(msgRes, category, id, 0);
    LevelName->setMessage(msgRes, category, id, 0);

    ulong number = pregameGetLevelNumberID(dInfo_c::m_startGameInfo.mWorld1, dInfo_c::m_startGameInfo.mLevel1);

    LevelNumShadow->setMessage(msgRes, BMG_CATEGORY_LEVEL_NAMES, number, 0);
    LevelNum->setMessage(msgRes, BMG_CATEGORY_LEVEL_NAMES, number, 0);

    nw4r::lyt::Picture *LevelSample;
    LevelSample = _this->mLayout.findPictureByName("LevelSample");

    // get preview name
    char tplName[64];
    sprintf(tplName, "/LevelSamples/%02d-%02d.tpl", dInfo_c::m_startGameInfo.mWorld1+1, dInfo_c::m_startGameInfo.mLevel1+1);
    int entrynum = DVDConvertPathToEntrynum(tplName);
    if (entrynum == -1) return;

    // load the file
    DVDFileInfo dvdHandle;
    bool fileLoaded = DVDFastOpen(entrynum, &dvdHandle);
    if (!fileLoaded) return;

    // allocate memory
    void* buffer = EGG::Heap::alloc(dvdHandle.size, 0x20, mHeap::g_archiveHeap);
    if (buffer == nullptr) return;
    
    // read file
    s32 length = DVDReadPrio(&dvdHandle, buffer, dvdHandle.size, 0, 2);
    if (length > 0) {
        LevelSample->GetMaterial()->GetTexMapAry()->ReplaceImage((TPLPalette *)buffer, 0, nullptr);
    }

    // unload file and free memory
    DVDClose(&dvdHandle);
    // TODO: figure out why this causes corrupt textures
    //EGG::Heap::free(buffer, mHeap::g_archiveHeap);
}

#endif