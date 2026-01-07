#include <kamek.h>
#include <new/bases/d_a_en_kakibon.hpp>
#include <new/new_profile.hpp>

CUSTOM_ACTOR_PROFILE(EN_KAKIBON, daEnKakibon_c, fProfile::EN_KURIBO, fProfile::DRAW_ORDER::EN_KURIBO, 0x12);

const char* l_KAKIBON_res [] = {"kakibon", NULL};
const dActorData_c c_KAKIBON_actor_data = {fProfile::EN_KAKIBON, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c l_KAKIBON_profile(&g_profile_EN_KAKIBON, "EN_KAKIBON", SpriteId::EN_KAKIBON, &c_KAKIBON_actor_data, l_KAKIBON_res);

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

bool daEnKakibon_c::checkForLedge(float xOffset) {
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