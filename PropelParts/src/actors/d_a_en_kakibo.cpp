#include <kamek.h>
#include <propelparts/bases/d_a_en_kakibo.hpp>
#include <propelparts/bases/d_custom_profile.hpp>

CUSTOM_ACTOR_PROFILE(EN_KAKIBO, daEnKakibo_c, fProfile::EN_KURIBO, fProfile::DRAW_ORDER::EN_KURIBO, 0x12);

const char* l_KAKIBO_res [] = {"kakibo", NULL};
const dActorData_c c_KAKIBO_actor_data = {fProfile::EN_KAKIBO, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c l_KAKIBO_profile(&g_profile_EN_KAKIBO, "EN_KAKIBO", CourseActor::EN_KAKIBO, &c_KAKIBO_actor_data, l_KAKIBO_res);

void daEnKakibo_c::executeState_Walk() {
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
        if (checkForLedge(2.5f) == false) { // Check for ledges
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

void daEnKakibo_c::createBodyModel() {
    mRes = dResMng_c::m_instance->getRes("kakibo", "g3d/kakibo.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("kakibo");
    mModel.create(mdl, &mAllocator, 0x227, 1, nullptr);
    setSoftLight_Enemy(mModel);

    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk");
    mAnmChr.create(mdl, resAnmChr, &mAllocator, 0);

    mResPat = mRes.GetResAnmTexPat("walk");
    mAnmTexPat.create(mdl, mResPat, &mAllocator, 0, 1);
    return;
}

bool daEnKakibo_c::isBgmSync() const {
    return false;
}

bool daEnKakibo_c::checkForLedge(float xOffset) {
    float xOffs[] = {xOffset, -xOffset};

    mVec3_c tileToCheck;
    tileToCheck.y = 4.0f + mPos.y;
    tileToCheck.z = mPos.z;
    tileToCheck.x = mPos.x + xOffs[mDirection];

    u32 unit = mBc.getUnitKind(tileToCheck.x, mPos.y - 2.0f, mLayer);

    if (((unit >> 0x10) & 0xFF) == 8) {
        return false;
    } else {
        float zeroFloat = 0.0f;
        bool result = mBc.checkGround(&tileToCheck, &zeroFloat, mLayer, 1, -1);
        if (((!result) || (tileToCheck.y <= zeroFloat)) || (zeroFloat <= mPos.y - 5.0f)) {
            return false;
        } else {
            return true;
        }
    }

    return false;
}