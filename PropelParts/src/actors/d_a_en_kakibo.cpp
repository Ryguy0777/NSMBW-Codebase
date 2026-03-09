#include <kamek.h>
#include <propelparts/bases/d_a_en_kakibo.hpp>
#include <propelparts/bases/d_custom_profile.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_game_com.hpp>

// Special thanks to Abood for decompiling NSMBU's Goombrat

CUSTOM_ACTOR_PROFILE(EN_KAKIBO, daEnKakibo_c, fProfile::EN_KURIBO, fProfile::DRAW_ORDER::EN_KURIBO, 0x12);

const char* l_KAKIBO_res [] = {"kakibo", NULL};
const dActorData_c c_KAKIBO_actor_data = {fProfile::EN_KAKIBO, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c l_KAKIBO_profile(&g_profile_EN_KAKIBO, "EN_KAKIBO", CourseActor::EN_KAKIBO, &c_KAKIBO_actor_data, l_KAKIBO_res);

void daEnKakibo_c::finalUpdate() {
    daEnKuribo_c::finalUpdate();

    // Update the "hair" on the Goombrat during a bah
    for (int i = 1; i < 5; i++) {
        char nodeName[6];
        sprintf(nodeName, "ha_%d", i);
        nw4r::g3d::ResNode ha = mModel.getResMdl().GetResNode(nodeName);
        ulong haId = ha.GetID();

        mVec3_c haScale(mHaScale, mHaScale, mHaScale);

        mMtx_c haLocalMtxAng;
        haLocalMtxAng.ZrotS(-mHaAngleZ);

        mMtx_c haLocalMtxScale;
        PSMTXScale(haLocalMtxScale, haScale.x, haScale.y, haScale.z);

        mMtx_c haLocalMtx;
        nw4r::math::MTX34Mult(&haLocalMtx, &haLocalMtxAng, &haLocalMtxScale);

        mMtx_c haWorldMtx;
        mModel.getNodeWorldMtx(haId, &haWorldMtx);

        nw4r::math::MTX34Mult(&haWorldMtx, &haWorldMtx, &haLocalMtx);
        nw4r::g3d::ScnMdlSimple *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mModel.getScn());
        nw4r::math::MTX34Copy(&scnMdl->GetWldMtxArray()[ha.GetMtxID()], &haWorldMtx);
    }
    
    return;
}

void daEnKakibo_c::initializeState_DieOther() {
    daEnKuribo_c::initializeState_DieOther();
    mHaAngleZ = 0;
    mHaScale = 1.0f;
    return;
}

void daEnKakibo_c::initializeState_DieFall() {
    dEn_c::initializeState_DieFall();
    mHaAngleZ = 0;
    mHaScale = 1.0f;
    return;
}

void daEnKakibo_c::initializeState_Walk() {
    setKakiboAnm(checkForLedge(36.0f) ? ANM_WALK0 : ANM_WALK1);
    setWalkSpeed();
    mAccelY = -0.1875;
    mSpeedMax.set(0.0f, -4.0f, 0.0f);
    return;
}

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
            if (!mIsWalk1) {
                setKakiboAnm(ANM_WALK1);
            }
            changeState(StateID_Turn);
        }
        if (dAudio::isBgmAccentSign(1)) {
            mBgmHoldTimer = 10;
        }
        calcHa();
    }
    // Turn if touching a wall
    if (mBc.mFlags & 0x15 << mDirection & 0x3f) {
        changeState(StateID_Turn);
    }
    if (checkForLedge(36.0f)) {
        setKakiboAnm(ANM_WALK0);
    } else {
        setKakiboAnm(ANM_WALK1);
    }
    WaterCheck(mPos, 1.0f);
    return;
}

void daEnKakibo_c::executeState_Turn() {
    playWalkAnm();
    calcSpeedY();
    posMove();

    if ((EnBgCheck() & 1) == 0) {
        if (mBc.isFoot() && (mInLiquid == false) && (mSpeed.y <= 0.0f)) {
            mFootPush2.x = mFootPush2.x + m_1eb.x;
        }
    } else {
        mSpeed.y = 0.0f;
        if (dAudio::isBgmAccentSign(1)) {
            mBgmHoldTimer = 10;
        }
        calcHa();
    }

    WaterCheck(mPos, 1.0f);

    // Face our new direction, and exit state when finished
    if (sLib::chaseAngle((short*)&mAngle.y, l_base_angleY[mDirection], 0x200)) {
        changeState(StateID_Walk);
    }

    if (checkForLedge(36.0f)) {
        setKakiboAnm(ANM_WALK0);
    } else {
        setKakiboAnm(ANM_WALK1);
    }
    return;
}

void daEnKakibo_c::createBodyModel() {
    mRes = dResMng_c::m_instance->getRes("kakibo", "g3d/kakibo.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("kakibo");
    mModel.create(mdl, &mAllocator, 0x227, 1, nullptr);
    setSoftLight_Enemy(mModel);

    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk0");
    mAnmChr.create(mdl, resAnmChr, &mAllocator, 0);

    mResPat = mRes.GetResAnmTexPat("walk0");
    mAnmTexPat.create(mdl, mResPat, &mAllocator, 0, 1);
    return;
}

void daEnKakibo_c::initialize() {
    mHaScale = 1.0f;
    return daEnKuribo_c::initialize();
}

void daEnKakibo_c::setTurnByEnemyHit() {
    mIsWalk1 = false;
    setKakiboAnm(ANM_WALK0);
    return daEnKuribo_c::setTurnByEnemyHit();
}

void daEnKakibo_c::setWalkAnm() {
    setKakiboAnm(ANM_WALK0);
    mAnmTexPat.setFrame(dGameCom::rndInt(180), 0);
    return;
}

bool daEnKakibo_c::isBgmSync() const {
    return false;
}

void daEnKakibo_c::setKakiboAnm(KAKIBO_ANIM_ID_e anmId) {
    if (mIsWalk1) {
        if (mAnmChr.isStop()) {
            mIsWalk1 = false;
            mKakiboAnm = ANM_WALK;
        }
    }
    if (mKakiboAnm != anmId) {
        switch (anmId) {
            default:
                break;
            case ANM_WALK0:
                if (!mIsWalk1) {
                    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk0");
                    mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
                    mModel.setAnm(mAnmChr, 10.0f);
                    mAnmChr.setFrame(0.0f);
                    mAnmChr.setRate(2.0f);

                    float prevPATFrame = mAnmTexPat.getFrame(0);
                    mResPat = mRes.GetResAnmTexPat("walk0");
                    mAnmTexPat.setAnm(mModel, mResPat, 0, m3d::FORWARD_LOOP);
                    mAnmTexPat.setFrame(prevPATFrame, 0);
                    mAnmTexPat.setRate(1.0f, 0);
                    mModel.setAnm(mAnmTexPat);

                    mKakiboAnm = anmId;
                }
                break;
            case ANM_WALK1:
                nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk1");
                mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_ONCE);
                mModel.setAnm(mAnmChr, 10.0f);
                mAnmChr.setFrame(0.0f);
                mAnmChr.setRate(2.0f);

                float prevPATFrame = mAnmTexPat.getFrame(0);
                mResPat = mRes.GetResAnmTexPat("walk1");
                mAnmTexPat.setAnm(mModel, mResPat, 0, m3d::FORWARD_LOOP);
                mAnmTexPat.setFrame(prevPATFrame, 0);
                mAnmTexPat.setRate(1.0f, 0);
                mModel.setAnm(mAnmTexPat);

                mKakiboAnm = anmId;

                mIsWalk1 = true;
                break;
        }
    }
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