#include <kamek.h>
#include <new/bases/d_a_lift_bg_rail.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_switch_flag_mng.hpp>

CUSTOM_ACTOR_PROFILE(AC_LIFT_BG_RAIL, daLiftBgRail_c, fProfile::EN_SLIP_PENGUIN, fProfile::DRAW_ORDER::EN_SLIP_PENGUIN);

const dActorData_c c_LIFT_BG_RAIL_actor_data = {fProfile::AC_LIFT_BG_RAIL, 8, -8, 0, 0, 0, 0, 0, 0, 0, 0, ACTOR_CREATE_GLOBAL};
dCustomProfile_c l_LIFT_BG_RAIL_profile(&g_profile_AC_LIFT_BG_RAIL, "AC_LIFT_BG_RAIL", SpriteId::AC_LIFT_BG_RAIL, &c_LIFT_BG_RAIL_actor_data);

bool daLiftBgRail_c::initMovement() {
    mRailIdx = (mParam >> 24) & 0xFF;
    mRailIndex = (mParam >> 16) & 0xFF;

    mRail.set(mRailIdx, &mRailPos, &mSpeed, &mSpeedF, mRailIndex, false);

    mRail.mBounce = (mParam >> 15) & 1;

    if (mRailIdx != 0) {
        mRailOffset = mPos - mRailPos;
    }

    return true;
};

void daLiftBgRail_c::executeMovement() {
    // Only move if event activated
    if (dSwitchFlagMng_c::checkEvent(mEventNums[1]-1) || mEventNums[1] == 0) {
        mRail.execute();
    }

    if (mRailIdx != 0) {
        mPos = mRailPos + mRailOffset;
    }

    if (mRail.mIdxCurr == mRail.mCount-1) {
        // We've reached the end of the path, activate the target event
        dSwitchFlagMng_c::m_instance->set(mEventNums[0]-1, 0, true, false, false, false);
    }
};