#include <kamek.h>
#include <propelparts/bases/d_a_en_kakibo.hpp>
#include <propelparts/bases/d_custom_profile.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_game_com.hpp>

// VERY special thanks to Abood for decompiling NSMBU's Goombrat and helping with the "should look down" functions

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
    if (mStateMgr.getOldStateID()->isEqual(StateID_Turn)) {
        setKakiboAnm(checkTurnSaka() ? ANM_WALK1 : ANM_WALK0);
    } else {
        setWalkAnm();
    }
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
        if (checkLedge(1.5f) == false) { // Check for ledges
            if (!mIsWalk1) {
                setKakiboAnm(ANM_WALK1);
            }
            changeState(StateID_Turn);
            return;
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
    if (checkTurnSaka()) {
        setKakiboAnm(ANM_WALK1);
    } else {
        setKakiboAnm(ANM_WALK0);
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

    if (checkTurnSaka()) {
        setKakiboAnm(ANM_WALK1);
    } else {
        setKakiboAnm(ANM_WALK0);
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

bool daEnKakibo_c::checkGround(float xDist) {
    float checkDist[] = { xDist, -xDist };

    s16 surfAngle = mBc.getSakaAngle(0);
    float sin = nw4r::math::SinIdx(surfAngle);
    float cos = nw4r::math::CosIdx(surfAngle);

    mVec2_c checkVec(checkDist[mDirection] * cos, checkDist[mDirection] * sin);
    mVec2_c checkPosBase(mPos.x + checkVec.x, mPos.y + checkVec.y);

    mVec3_c p0(checkPosBase.x, checkPosBase.y + 10.0f, mPos.z);
    mVec3_c p1(checkPosBase.x, checkPosBase.y - 6.0f, mPos.z);

    float groundY;
    bool found = mBc.checkGround(&p0, &groundY, mLayer, l_Ami_Line[mAmiLayer], -1);
    float dist = p0.y - groundY;
    if (found && dist <= p0.y - p1.y) {
        return true;
    }
    return false;
}

float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

bool daEnKakibo_c::checkTurnSaka() {
    if (!mBc.checkFootEnm()) {
        return false;
    }
    if (checkGround(30.0f)) {
        return false;
    }
    const float c_check_dist_x = 36.0f;
    const float c_check_dist_a[] = { c_check_dist_x, -c_check_dist_x };
    const float c_check_dist_b = 4.0f;

    s16 angleA = mBc.getSakaAngle(0);
    float sinA = nw4r::math::SinIdx(angleA);
    float cosA = nw4r::math::CosIdx(angleA);

    s16 angleB = angleA - 0x4000;
    float sinB = nw4r::math::SinIdx(angleB);
    float cosB = nw4r::math::CosIdx(angleB);

    mVec2_c checkVecA(
        c_check_dist_a[mDirection] * cosA,
        c_check_dist_a[mDirection] * sinA
    );

    mVec2_c checkVecB(
        c_check_dist_b * cosB,
        c_check_dist_b * sinB
    );

    mVec2_c checkVec = checkVecA + checkVecB;

    mVec2_c p0(mPos.x + checkVec.x, mPos.y + checkVec.y);
    mVec2_c p1(mPos.x + checkVecB.x, mPos.y + checkVecB.y);

    int numSteps = std::ceil(c_check_dist_x / 4);

    bool  hasPrev = false;
    float prevLineY = 0.f;
    float prevGndY = 0.f;
    short prevAng = 0;
    float prevX = 0.f;

    for (int i = 0; i <= numSteps; ++i) {
        float t  = static_cast<float>(i) / numSteps;
        float px = Lerp(p0.x, p1.x, t);
        float py = Lerp(p0.y, p1.y, t);
        mVec3_c p(px, py, 0.0f);

        float groundY = 0.0f;
        short groundAngle = 0;
        if (!mBc.checkGroundAngle(&p, &groundY, &groundAngle, mLayer, 1, -1, nullptr, 0) || groundY > py) {
            hasPrev = false;
            continue;
        }
        if (groundY == py) {
            return false;
        }

        if (hasPrev) {
            float dx = px - prevX;

            float projPrev = prevGndY + (nw4r::math::SinIdx(prevAng) / nw4r::math::CosIdx(prevAng)) * dx;
            if (projPrev >= py) {
                return false;
            }

            float projCur = groundY - (nw4r::math::SinIdx(groundAngle) / nw4r::math::CosIdx(groundAngle)) * dx;
            if (projCur >= prevLineY) {
                return false;
            }
        }

        hasPrev = true;
        prevLineY = py;
        prevGndY = groundY;
        prevAng = groundAngle;
        prevX = px;
    }

    return true;
}

bool daEnKakibo_c::checkLedge(float xOffset) {
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