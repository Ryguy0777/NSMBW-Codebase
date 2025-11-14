#include <kamek.h>
#include <new/game_config.h>

#ifdef LEVEL_INFO_UTILS_ENABLED
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_multi_course_select.hpp>
#include <constants/message_list.h>
#include <new/bases/d_level_info.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>

// Allow Recommended Courses page to hide buttons
kmWriteNop(0x8079DEEC);
kmWriteNop(0x8079DEF0);

// Types: 0 = world, 1 = recommended
void dMultiCourseSelect_c::loadTitleFromLI(int tb, int type) {
    const wchar_t *worldName;
    if (type == 1) {
        bool isCoinBattle = (dInfo_c::mGameFlag & dInfo_c::GAME_FLAG_IS_COIN_COURSE);
        worldName = getWorldName(((isCoinBattle) ? 0xFF : 0xFE)-1);
    } else {
        worldName = getWorldName(dInfo_c::m_instance->mDisplayCourseWorld-1);
    }

    mpTextBoxes[tb]->SetString(worldName, 0);
}

// Update page titles
kmBranchDefCpp(0x80799B30, NULL, void, dMultiCourseSelect_c *this_, int tb) {
    dInfo_c::m_instance->mScissorPane = this_->mpNullPanes[this_->N_scissor_00];
    dInfo_c::m_instance->mDrawInfo = this_->mLayout.GetDrawInfo();
    dInfo_c::m_instance->mScissorInfoIdx = (tb != 1) ? 2 : 1;
    int textBoxIdx = tb != 1;
    int page = this_->mCurrentPage;
    int type = 0;

    if (page == 10) {
        type = 2;
    } else if (page == 0) {
        type = 1;
    } else {
        type = 0;
    }

    if (type == 2) {
        MsgRes_c *msgRes = dMessage_c::getMesRes();
        this_->mpTextBoxes[textBoxIdx]->setMessage(msgRes, BMG_CATEGORY_MAIN_MENU, MSG_MAIN_MENU_FAVORITES, 0);
    } else {
        dInfo_c::m_instance->mDisplayCourseWorld = page;
        this_->loadTitleFromLI(textBoxIdx, type);
    }
    this_->colorTitleText(textBoxIdx, type);
}

void GetLevelID(dMultiCourseSelect_c *this_, u8 *world, u8 *level, int button) {
    int page = this_->mCurrentPage;
    if (button > 9) {
        button -= 10;
    }

    if (page == 0) {
        *world = getRecommendedWorld((dInfo_c::mGameFlag & dInfo_c::GAME_FLAG_IS_COIN_COURSE) == 0, button);
        *level = getRecommendedLevel((dInfo_c::mGameFlag & dInfo_c::GAME_FLAG_IS_COIN_COURSE) == 0, button);
    }

    else if (page == 10) {
        *world = this_->mFavoritedCourses[button].mWorldNum;
        *level = this_->mFavoritedCourses[button].mLevelNum;
    }

    else {
        u8 worldNum = getWorldForButton(page-1, button);
        *world = worldNum;
        *level = dGameCom::getCourseNum(worldNum, button);
    }
}

// Returns a level ID
kmBranchDefCpp(0x8079C410, NULL, void, dMultiCourseSelect_c *this_, u8 *world, u8 *level, int button) {
    GetLevelID(this_, world, level, button);
}

// Set button world/level numbers
kmBranchDefCpp(0x80799E80, NULL, void, dMultiCourseSelect_c *this_, int button, int maxButtonCount) {
    u8 world, level;
    while (true) {
        if (maxButtonCount <= button) {
            return;
        }
        this_->mpContents[button]->mPreviousPage = this_->mCurrentPage;
        GetLevelID(this_, &world, &level, button);

        this_->mpContents[button]->mWorldNumber = world;
        this_->mpContents[button]->mLevelNumber = level;
        button += 1;
    }
}
#endif