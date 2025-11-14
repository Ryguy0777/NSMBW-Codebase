#include <kamek.h>
#include <new/game_config.h>

#ifdef LEVEL_INFO_UTILS_ENABLED
#include <game/bases/d_info.hpp>
#include <game/bases/d_multi_course_select_contents.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>

// Update button info
kmBranchDefCpp(0x8079E0C8, 0x8079E178, void, dMultiCourseSelectContents_c *this_) {
    this_->mpTextBoxes[this_->T_corseNum_00]->SetVisible(false);
    this_->mpTextBoxes[this_->T_corsePic_00]->SetVisible(false);

    this_->mpTextBoxes[this_->T_W_corseNum_00]->SetVisible(true);
    this_->mpTextBoxes[this_->T_W_corseNum_01]->SetVisible(true);
    this_->mpTextBoxes[this_->T_W_corsePic_00]->SetVisible(false);

    dLevelInfo_c::entry_s *level = dLevelInfo_c::m_instance.getEntryFromSlotID(this_->mWorldNumber, this_->mLevelNumber);
    if (level) {
        const wchar_t *worldName = getWorldNumber(level->mDisplayWorld);
        ulong number = getLevelNumberIdx(level->mDisplayWorld, level->mDisplayLevel, level->mWorldSlot, level->mLevelSlot, true);
        MsgRes_c *msgRes = dMessage_c::getMesRes();

        this_->mpTextBoxes[this_->T_W_corseNum_00]->SetString(worldName, 0);
        if (level->mDisplayLevel > 19) {
            this_->mpTextBoxes[this_->T_W_corseNum_02]->SetVisible(false);
            this_->mpTextBoxes[this_->T_W_corsePic_00]->SetVisible(true);
            this_->mpTextBoxes[this_->T_W_corsePic_00]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, number, 0);
        } else {
            this_->mpTextBoxes[this_->T_W_corsePic_00]->SetVisible(false);
            this_->mpTextBoxes[this_->T_W_corseNum_02]->SetVisible(true);
            this_->mpTextBoxes[this_->T_W_corseNum_02]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, number, 0);
        }
    } else {
        this_->mpTextBoxes[this_->T_W_corseNum_00]->SetString(getWorldNumber(0), 0);
        this_->mpTextBoxes[this_->T_W_corsePic_00]->SetVisible(false);
        this_->mpTextBoxes[this_->T_W_corseNum_02]->SetVisible(true);
        this_->mpTextBoxes[this_->T_W_corseNum_02]->SetString(getLevelNumber(0), 0);
    }
}

// Return a preview texture filename
kmBranchDefCpp(0x8079DE80, NULL, const char *, dMultiCourseSelectContents_c *this_, int world, int level) {
    static char buf[32];
    if (level == 19) {
        sprintf(buf, "msk_coin%02d.tpl", world+1);
    } else if (level > 19) {
        const char *string = "1"; // Just in case we fail, it should find the X-1 preview
        switch (level) {
            case 20: string = "obake"; break;
            case 21: string = "toride"; break;
            case 22: string = "toride2"; break; //new, tower 2
            case 23: string = "shiro"; break;
            case 24: string = "shiro2"; break; //new, castle 2
            case 37: string = "hikousen"; break;
        }
        sprintf(buf, "im_W%d_%s.tpl", world+1, string);
    } else {
        sprintf(buf, "im_W%d_%d.tpl", world+1, level+1);
    }
    return buf;
}
#endif