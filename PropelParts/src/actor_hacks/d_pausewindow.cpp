#include <kamek.h>
#include <new/game_config.h>

#ifdef LEVEL_INFO_UTILS_ENABLED
#include <game/bases/d_info.hpp>
#include <game/bases/d_pausewindow.hpp>
#include <new/bases/d_level_info.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>

#ifdef NEWER_PAUSE_MENU
const char *newerPauseMenuName = "pauseMenu_newer/pauseMenu.arc";
kmBranchDefAsm(0x8015A790, 0x8015A794) {
    lis r4, newerPauseMenuName@h
    ori r4, r4, newerPauseMenuName@l
    lwz r4, 0(r4)
    blr
}
#endif

kmBranchDefCpp(0x8015AC60, NULL, void, Pausewindow_c *this_) {
    dLevelInfoUtils_c::PauseWindowLevelInfo(this_);
}

void dLevelInfoUtils_c::PauseWindowLevelInfo(Pausewindow_c *this_) {
    MsgRes_c *msgRes = dMessage_c::getMesRes();

    int csPage = dInfo_c::m_instance->mCourseSelectPageNum;
    int csIdx = dInfo_c::m_instance->mCourseSelectIndexInPage;
    if (csIdx > 9) {
        csIdx -= 10;
    }

    this_->mpNullPanes[Pausewindow_c::N_osusumeText_00]->SetVisible(false);
    this_->mpNullPanes[Pausewindow_c::N_multiText_00]->SetVisible(false);

    // Show a coin if we're in a Coin Battle course
    bool isNotCoinCourse = (((dInfo_c::mGameFlag & dInfo_c::GAME_FLAG_IS_COIN_COURSE) == 0) || (csPage != 0)) || (csIdx > 4);
    this_->mpPicturePanes[Pausewindow_c::P_coin_00]->SetVisible(!isNotCoinCourse);
    this_->mpTextBoxes[Pausewindow_c::T_worldNum_00]->SetVisible(isNotCoinCourse);

    dLevelInfo_c::entry_s *level = dLevelInfo_c::m_instance.getEntryFromSlotID(dInfo_c::m_startGameInfo.mWorld1, dInfo_c::m_startGameInfo.mLevel1);

#ifdef NEWER_PAUSE_MENU
    LytTextBox_c
        *LevelNumShadow, *LevelNum,
        *LevelNameShadow, *LevelName;

    LevelNumShadow = (LytTextBox_c *)this_->mLayout.findTextBoxByName("LevelNumShadow");
    LevelNum = (LytTextBox_c *)this_->mLayout.findTextBoxByName("LevelNum");
    LevelNameShadow = (LytTextBox_c *)this_->mLayout.findTextBoxByName("LevelNameShadow");
    LevelName = (LytTextBox_c *)this_->mLayout.findTextBoxByName("LevelName");

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
#else
    if (level) {
        const wchar_t *worldNum = getWorldNumber(level->mDisplayWorld);
        ulong number = getLevelNumberIdx(level->mDisplayWorld, level->mDisplayLevel, level->mWorldSlot, level->mLevelSlot, false);

        this_->mpTextBoxes[Pausewindow_c::T_worldNum_00]->SetString(worldNum, 0);
        if (level->mDisplayLevel > 19) {
            this_->mpTextBoxes[Pausewindow_c::T_corsePic_00]->SetVisible(true);
            this_->mpTextBoxes[Pausewindow_c::T_corseNum_00]->SetVisible(false);
            this_->mpTextBoxes[Pausewindow_c::T_corsePic_00]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, number, 0);
        } else {
            this_->mpTextBoxes[Pausewindow_c::T_corsePic_00]->SetVisible(false);
            this_->mpTextBoxes[Pausewindow_c::T_corseNum_00]->SetVisible(true);
            this_->mpTextBoxes[Pausewindow_c::T_corseNum_00]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, number, 0);
        }
    } else {
        this_->mpTextBoxes[Pausewindow_c::T_corsePic_00]->SetVisible(false);
        this_->mpTextBoxes[Pausewindow_c::T_corseNum_00]->SetVisible(true);
        this_->mpTextBoxes[Pausewindow_c::T_worldNum_00]->SetString(getWorldNumber(0), 0);
        this_->mpTextBoxes[Pausewindow_c::T_corseNum_00]->SetString(getLevelNumber(0), 0);
    }
#endif
}

#endif