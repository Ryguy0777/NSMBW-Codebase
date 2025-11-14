#include <new/game_config.h>

#ifdef LEVEL_INFO_UTILS_ENABLED
#include <kamek.h>

#include <constants/message_list.h>
#include <game/bases/d_pregame_lyt.hpp>
#include <game/bases/d_info.hpp>
#include <new/constants/message_list.h>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <new/level_info_utils.hpp>

#ifdef NEWER_PREGAME
#include <game/mLib/m_heap.hpp>
#include <lib/revolution/TPL.h>

// Change layout name
const char *newerPregameName = "preGame_newer/preGame.arc";
kmBranchDefAsm(0x80b6c074, 0x80b6c078) {
    lis r4, newerPregameName@h
    ori r4, r4, newerPregameName@l
    lwz r4, 0(r4)
    blr
}

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

void *pregameTPLbuffer = nullptr;
#endif

kmBranchDefCpp(0x80B6BDD0, NULL, void, dPreGameLyt_c *_this) {
    MsgRes_c *msgRes = dMessage_c::getMesRes();
    dLevelInfo_c::entry_s *level = dLevelInfo_c::m_instance.getEntryFromSlotID(dInfo_c::m_startGameInfo.mWorld1, dInfo_c::m_startGameInfo.mLevel1);

    int bgPaneIdx = dInfo_c::m_startGameInfo.mWorld1;
    if (bgPaneIdx > 8)
        bgPaneIdx = 9;

    _this->mpPicturePanes[dPreGameLyt_c::P_W1_00+bgPaneIdx]->SetVisible(true);

#ifdef NEWER_PREGAME
    LytTextBox_c
        *LevelNumShadow, *LevelNum,
        *LevelNameShadow, *LevelName;

    LevelNumShadow = (LytTextBox_c *)_this->mLayout.findTextBoxByName("LevelNumShadow");
    LevelNum = (LytTextBox_c *)_this->mLayout.findTextBoxByName("LevelNum");
    LevelNameShadow = (LytTextBox_c *)_this->mLayout.findTextBoxByName("LevelNameShadow");
    LevelName = (LytTextBox_c *)_this->mLayout.findTextBoxByName("LevelName");

    if (level) {
        const wchar_t *name = getLevelName(level->mDisplayWorld, level->mDisplayLevel);
        LevelNameShadow->SetString(name, 0);
        LevelName->SetString(name, 0);

        // grab the level number
        ulong number = getLevelNumberIdx(level->mDisplayWorld, level->mDisplayLevel, level->mWorldSlot, level->mLevelSlot, false);
        LevelNumShadow->setMessage(msgRes, BMG_CATEGORY_LEVEL_NAMES, number, 0);
        LevelNum->setMessage(msgRes, BMG_CATEGORY_LEVEL_NAMES, number, 0);
    } else {
        LevelNumShadow->setMessage(msgRes, BMG_CATEGORY_LEVEL_NAMES, 1, 0);
        LevelNum->setMessage(msgRes, BMG_CATEGORY_LEVEL_NAMES, 1, 0);
        LevelNameShadow->SetString(getLevelName(0, 0), 0);
        LevelName->SetString(getLevelName(0, 0), 0);
    }
#else // Retail Pre Game
    _this->mpTextBoxes[dPreGameLyt_c::T_world_00]->setMessage(msgRes, BMG_CATEGORY_PRE_GAME, MSG_PRE_GAME_WORLD, 0);

    if (level) {
        const wchar_t *worldNum = getWorldNumber(level->mDisplayWorld);
        ulong number = getLevelNumberIdx(level->mDisplayWorld, level->mDisplayLevel, level->mWorldSlot, level->mLevelSlot, false);

        _this->mpTextBoxes[dPreGameLyt_c::T_worldNum_00]->SetString(worldNum, 0);
        if (level->mDisplayLevel > 19) {
            _this->mpTextBoxes[dPreGameLyt_c::T_pictureFont_00]->SetVisible(true);
            _this->mpTextBoxes[dPreGameLyt_c::T_corseNum_00]->SetVisible(false);
            _this->mpTextBoxes[dPreGameLyt_c::T_pictureFont_00]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, number, 0);
        } else {
            _this->mpTextBoxes[dPreGameLyt_c::T_pictureFont_00]->SetVisible(false);
            _this->mpTextBoxes[dPreGameLyt_c::T_corseNum_00]->SetVisible(true);
            _this->mpTextBoxes[dPreGameLyt_c::T_corseNum_00]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, number, 0);
        }
    } else {
        _this->mpTextBoxes[dPreGameLyt_c::T_pictureFont_00]->SetVisible(false);
        _this->mpTextBoxes[dPreGameLyt_c::T_corseNum_00]->SetVisible(true);
        _this->mpTextBoxes[dPreGameLyt_c::T_worldNum_00]->SetString(getWorldNumber(0), 0);
        _this->mpTextBoxes[dPreGameLyt_c::T_corseNum_00]->SetString(getLevelNumber(0), 0);
    }
#endif

#ifdef NEWER_PREGAME
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
    pregameTPLbuffer = EGG::Heap::alloc(dvdHandle.size, 0x20, mHeap::g_archiveHeap);
    if (pregameTPLbuffer == nullptr) return;
    
    // read file
    s32 length = DVDReadPrio(&dvdHandle, pregameTPLbuffer, dvdHandle.size, 0, 2);
    if (length > 0) {
        LevelSample->GetMaterial()->GetTexMapAry()->ReplaceImage((TPLPalette *)pregameTPLbuffer, 0, nullptr);
    }

    // unload file
    DVDClose(&dvdHandle);
#endif
}

#ifdef NEWER_PREGAME
// remove the tpl from memory
kmBranchDefCpp(0x80b6c580, NULL, bool, dPreGameLyt_c *_this) {
    if (pregameTPLbuffer != nullptr) {
        EGG::Heap::free(pregameTPLbuffer, mHeap::g_archiveHeap);
    }
    return _this->mLayout.doDelete();
}
#endif

#endif