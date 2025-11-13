#include <kamek.h>
#include <new/game_config.h>

#ifdef LEVEL_INFO_UTILS_ENABLED
#include <game/bases/d_CourseSelectGuide.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>

kmBranchDefCpp(0x80010B50, NULL, void, dCourseSelectGuide_c *this_, dWmLib::PointType_e type) {
    this_->CourseSelectSet(type);
}

void dCourseSelectGuide_c::CourseSelectSet(dWmLib::PointType_e type) {
    if (type == dWmLib::POINT_TYPE_PATH) {
        mCourseNo = 0xfffffffe;
    } else if (type == dWmLib::POINT_TYPE_INTERSECTION) {
        // Get a dot
        mpTextBoxes[T_cSelect_pic]->SetVisible(true);
        mpTextBoxes[T_cSelect_00]->SetVisible(false);

        MsgRes_c *msgRes = dMessage_c::getMesRes();
        mpTextBoxes[T_cSelect_pic]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, 0x10, 0);
        dLevelInfo_c::entry_s *liWorld = dLevelInfo_c::m_instance.getEntryFromSlotID(mWorldNo, 38);
        if (liWorld) {
            const wchar_t *worldNum = getWorldNumber(liWorld->mDisplayWorld);
            mpTextBoxes[T_worldNum_00]->SetString(worldNum, 0);
        } else {
            mpTextBoxes[T_worldNum_00]->SetString(getWorldNumber(0), 0);
        }
    } else {
        if ((type - 3) < 2) {
            type = (dWmLib::PointType_e)(mCourseNo + 1);
        } else if (type == dWmLib::POINT_TYPE_START_NODE) {
            mCourseType = dWmLib::COURSE_TYPE_INVALID;
        } else {
            mCourseType = dWmLib::COURSE_TYPE_JUNCTION;
        }

        mRestAlphaTarget = 0xFF;

        // Hide the lives counter for Peach's Castle
        if (mCourseType == dWmLib::COURSE_TYPE_PEACH_CASTLE) {
            mRestAlphaTarget = 0;
        }

        mpTextBoxes[T_cSelect_pic]->SetVisible(false);
        mpTextBoxes[T_cSelect_00]->SetVisible(true);

        dLevelInfo_c::entry_s *level = dLevelInfo_c::m_instance.getEntryFromSlotID(mWorldNo, mCourseNo);
        if (level) {
            const wchar_t *worldName = getWorldNumber(level->mDisplayWorld);
            ulong number = getLevelNumberIdx(level->mDisplayWorld, level->mDisplayLevel, level->mWorldSlot, level->mLevelSlot, false);
            MsgRes_c *msgRes = dMessage_c::getMesRes();

            mpTextBoxes[T_worldNum_00]->SetString(worldName, 0);
            if (level->mDisplayLevel > 19) {
                mpTextBoxes[T_cSelect_pic]->SetVisible(true);
                mpTextBoxes[T_cSelect_00]->SetVisible(false);
                mpTextBoxes[T_cSelect_pic]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, number, 0);
            } else {
                mpTextBoxes[T_cSelect_pic]->SetVisible(false);
                mpTextBoxes[T_cSelect_00]->SetVisible(true);
                mpTextBoxes[T_cSelect_00]->setMessage(msgRes, BMG_CATEGORY_LEVEL_ICONS, number, 0);
            }
        } else {
            mpTextBoxes[T_cSelect_pic]->SetVisible(false);
            mpTextBoxes[T_cSelect_00]->SetVisible(true);
            mpTextBoxes[T_cSelect_00]->SetString(getWorldNumber(0), 0);
            mpTextBoxes[T_worldNum_00]->SetString(getWorldNumber(0), 0);
        }
    }
}
#endif