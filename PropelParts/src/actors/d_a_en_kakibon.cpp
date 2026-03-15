#include <kamek.h>
#include <propelparts/bases/d_a_en_kakibon.hpp>
#include <propelparts/bases/d_custom_profile.hpp>

CUSTOM_ACTOR_PROFILE(EN_KAKIBON, daEnKakibon_c, fProfile::EN_KURIBO, fProfile::DRAW_ORDER::EN_KURIBO, 0x12);

const char* l_KAKIBON_res [] = {"kakibon", NULL};
const dActorData_c c_KAKIBON_actor_data = {fProfile::EN_KAKIBON, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c l_KAKIBON_profile(&g_profile_EN_KAKIBON, "EN_KAKIBON", CourseActor::EN_KAKIBON, &c_KAKIBON_actor_data, l_KAKIBON_res);

void daEnKakibon_c::executeState_Walk() {
    // Update model & do speed calculations
    playWalkAnm();
    calcSpeedY();
    posMove();
    // Finish turning if not facing a direction
    sLib::chaseAngle((short*)&mAngle.y, l_base_angleY[mDirection], 0x800);

    if ((EnBgCheck() & 1) == 0) { // Not touching a tile
        // Related to walking speed in water? not sure
        if (mBc.isFoot() && (mInLiquid == false) && (mSpeed.y <= 0.0f)) {
            mFootPush2.x = mFootPush2.x + m_1eb.x;
        }
    } else { // Touching a tile
        mFootPush2.x = 0.0f;
        mSpeed.y = 0.0f;
        if (isOnEnLiftRemoconTrpln()) {
            changeState(StateID_TrplnJump);
        }
        if (checkLedge(1.5f) == false) { // Check for ledges
            changeState(StateID_Turn);
        }
    }
    // Turn if touching a wall
    if (mBc.mFlags & 0x15 << mDirection & 0x3f) {
        changeState(StateID_Turn);
    }
    WaterCheck(mPos, 1.0f);
    return;
}

void daEnKakibon_c::createBodyModel() {
    mRes = dResMng_c::m_instance->getRes("kakibon", "g3d/kakibon.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("kakibon");
    mModel.create(mdl, &mAllocator, 0x227, 1, nullptr);
    setSoftLight_Enemy(mModel);

    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk");
    mAnmChr.create(mdl, resAnmChr, &mAllocator, 0);
    return;
}

bool daEnKakibon_c::isBgmSync() const {
    return false;
}

bool daEnKakibon_c::checkLedge(float xOffset) {
    float xOffs[] = {xOffset, -xOffset};

    mVec3_c groundCheckPos(mPos.x + xOffs[mDirection], mPos.y + 4.0f, mPos.z);
    float groundY;
    bool found = mBc.checkGround(&groundCheckPos, &groundY, mLayer, l_Ami_Line[mAmiLayer], -1);
    float dist = groundCheckPos.y - groundY;
    if (found && dist <= groundCheckPos.y - mPos.y + 5.0f) {
        return true;
    }
    return false;
}