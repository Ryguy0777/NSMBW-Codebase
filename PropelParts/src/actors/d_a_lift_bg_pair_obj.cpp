#include <kamek.h>
#include <propelparts/bases/d_a_lift_bg_pair_obj.hpp>
#include <propelparts/bases/d_custom_profile.hpp>
#include <game/bases/d_a_en_pair_obj_parent.hpp>

CUSTOM_ACTOR_PROFILE(AC_LIFT_BG_PAIR_OBJ, daLiftBgPairObj_c, fProfile::EN_PAIR_OBJ_CHILD, fProfile::DRAW_ORDER::EN_PAIR_OBJ_CHILD);

const dActorData_c c_LIFT_BG_PAIR_OBJ_actor_data = {fProfile::AC_LIFT_BG_PAIR_OBJ, 8, -8, 0, 0, 0, 0, 0, 0, 0, 0, ACTOR_CREATE_GLOBAL};
dCustomProfile_c l_LIFT_BG_PAIR_OBJ_profile(&g_profile_AC_LIFT_BG_PAIR_OBJ, "AC_LIFT_BG_PAIR_OBJ", CourseActor::AC_LIFT_BG_PAIR_OBJ, &c_LIFT_BG_PAIR_OBJ_actor_data);

bool daLiftBgPairObj_c::initMovement() {
    mPairObjIdx = (mParam >> 24) & 0xFF;

    daEnPairObjParent_c *controller = (daEnPairObjParent_c *)findController();
    if (controller == nullptr) {
        return false;
    } else {
        mControllerID = controller->mUniqueID;
        mOrigin = mVec2_c(mPos.x, mPos.y);
        if (controller->mStateMgr.getStateID() == &daEnPairObjParent_c::StateID_Vanish) {
            setVisible(false);
        }
        return true;
    }

    return true;
};

void daLiftBgPairObj_c::executeMovement() {
    daEnPairObjParent_c *controller = (daEnPairObjParent_c *)fManager_c::searchBaseByID(mControllerID);
    if (controller->mVertical) {
        mPos.y = mOrigin.y + controller->mOffsetPos;
    } else {
        mPos.x = mOrigin.x + controller->mOffsetPos;
    }

    if (controller->mStateMgr.getStateID() == &daEnPairObjParent_c::StateID_Vanish) {
        if (!mIsHidden) {
            mBg.release();
            mIsHidden = true;
        }
        setVisible(false);
    } else {
        if (mIsHidden) {
            mBg.entry();
            mIsHidden = false;
        }
    }
};

fBase_c *daLiftBgPairObj_c::findController() {
    fBase_c *controller = nullptr;
    do {
        controller = fManager_c::searchBaseByProfName(fProfile::EN_PAIR_OBJ_PARENT, controller);
        if (controller == nullptr) {
            return nullptr;
        }
    } while (mPairObjIdx != (controller->mParam >> 0x18 & 0xFF));
    return controller;
}