#include <kamek.h>
#include <new/game_config.h>

#ifdef LEVEL_INFO_UTILS_ENABLED
#include <game/bases/d_model_play_date.hpp>
#include <new/bases/d_level_info.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>

// Externs for ASM calls
extern "C" void loadInfo(dModelPlayDate_c *);

kmBranchDefAsm(0x80794468, 0x807945A8) {
    or r3, r30, r30
    bl loadInfo
    blr
}

void loadInfo(dModelPlayDate_c *this_) {
    dLevelInfo_c::entry_s *level = dLevelInfo_c::m_instance.getEntryFromSlotID(this_->mWorldNum, this_->mCourseNum);
    if (level) {
        const wchar_t *worldName = getWorldNumber(level->mDisplayWorld);
        ulong number = getLevelNumberIdx(level->mDisplayWorld, level->mDisplayLevel, true);
        MsgRes_c *msgRes = dMessage_c::getMesRes();

        this_->mpTextBoxes[dModelPlayDate_c::T_worldNum_00]->SetString(worldName, 0);
        if (level->mDisplayLevel > 19) {
            this_->mpTextBoxes[dModelPlayDate_c::T_worldCourse_00]->SetVisible(false);
            this_->mpTextBoxes[dModelPlayDate_c::T_pictureFont_00]->SetVisible(true);
            this_->mpTextBoxes[dModelPlayDate_c::T_pictureFont_00]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, number, 0);
        } else {
            this_->mpTextBoxes[dModelPlayDate_c::T_pictureFont_00]->SetVisible(false);
            this_->mpTextBoxes[dModelPlayDate_c::T_worldCourse_00]->SetVisible(true);
            this_->mpTextBoxes[dModelPlayDate_c::T_worldCourse_00]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, number, 0);
        }
    } else {
        this_->mpTextBoxes[dModelPlayDate_c::T_worldNum_00]->SetString(getWorldNumber(0), 0);
        this_->mpTextBoxes[dModelPlayDate_c::T_pictureFont_00]->SetVisible(false);
        this_->mpTextBoxes[dModelPlayDate_c::T_worldCourse_00]->SetVisible(true);
        this_->mpTextBoxes[dModelPlayDate_c::T_worldCourse_00]->SetString(getLevelNumber(0), 0);
    }
}
#endif