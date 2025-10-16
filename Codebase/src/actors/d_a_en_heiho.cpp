#include <kamek.h>
#include <new/bases/d_a_en_heiho.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_audio.hpp>
#include <constants/sound_list.h>

CUSTOM_ACTOR_PROFILE(EN_HEIHO, daEnHeiho_c, fProfile::EN_KURIBO, fProfile::DRAW_ORDER::EN_KURIBO, 0x12);

STATE_DEFINE(daEnHeiho_c, Walk);
STATE_DEFINE(daEnHeiho_c, Turn);
STATE_DEFINE(daEnHeiho_c, Sleep);
STATE_DEFINE(daEnHeiho_c, Jump);
STATE_DEFINE(daEnHeiho_c, Dizzy);
STATE_DEFINE(daEnHeiho_c, Idle);

const char* heihoArcList[] = {"heiho", NULL};
const SpriteData heihoSpriteData = {fProfile::EN_HEIHO, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c heihoProfile(&g_profile_EN_HEIHO, "EN_HEIHO", SpriteId::EN_HEIHO, &heihoSpriteData, heihoArcList);

dCc_c::CcData_s l_heiho_cc = {
    0.0,                        // mOffsetX
    10.0,                       // mOffsetY
    8.0,                        // mWidth
    10.0,                       // mHeight
    dCc_c::CAT_ENTITY,          // mCategory
    0,                          // mAttackCategory
    0x6F,                       // mCategoryInteract
    0xFFBAFFFE,                 // mAttackCategoryInteract     
    0,                          // mFlag
    &dEn_c::normal_collcheck,   // mCallback
};

int daEnHeiho_c::create() {
    // setup our model
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);
    mRes = dResMng_c::m_instance->mRes.getRes("heiho", "g3d/heiho.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("heiho");
    mHeihoModel.create(mdl, &mAllocator, 0x227, 1, nullptr);
    dActor_c::setSoftLight_Enemy(mHeihoModel);

    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("idle");
    mAnmChr.create(mdl, resAnmChr, &mAllocator, 0);
    playChrAnim("idle", m3d::FORWARD_LOOP, 0.0, 2.0);

    mResPat = mRes.GetResAnmTexPat("color");
    mAnmTexPat.create(mdl, mResPat, &mAllocator, 0, 1);
    mAnmTexPat.setAnm(mHeihoModel, mResPat, 0, m3d::FORWARD_ONCE);
    mAnmTexPat.setRate(0.0f, 0);
    mHeihoModel.setAnm(mAnmTexPat);

    mAllocator.adjustFrmHeap();

    // set y acceleration and max speed for gravity
    mAccelY = -0.1875;
    mSpeedMax.y = -4.0;

    // register the cc data (hitbox)
    mCc.registerCc(this, &l_heiho_cc);
    mCc.entry();

    // assign settings variables
    mBackFence = (mParam >> 0x10 & 1);

    mType = (HEIHO_TYPE_e)(mParam >> 28 & 0xF);
    mColor = mParam >> 24 & 0xF;
    mHealth = mParam >> 17 & 1;
    mDistance = mParam >> 12 & 0xF;
    mSpawnDir = (mParam >> 8 & 1)^1;

    mAnmTexPat.setFrame(mColor, 0);

    // mCenterOffs is used to set the "center" of the actor
    // for yoshi tongue and dieFall
    mCenterOffs = mVec3_c(0.0, 12.0, 0.0);

    // set size for model culling
    mVisibleAreaSize.x = 28.0;
    mVisibleAreaSize.y = 32.0;
    mVisibleAreaOffset.x = 0.0;
    mVisibleAreaOffset.y = 12.0;

    // set yoshi eating behavior
    mEatBehaviour = EAT_TYPE_EAT_PERMANENT;

    float zPositions[2] = {1500.0, -2500.0};
    mPos.z = zPositions[mBackFence];

    // tile sensors
    static const dBcSensorLine_c below(-4<<12, 4<<12, 0<<12);
    static const dBcSensorPoint_c above(0<<12, 20<<12);
    static const dBcSensorLine_c adjacent(5<<12, 10<<12, 8<<12);

    mBc.set(this, (dBcSensor_c*)&below, (dBcSensor_c*)&above, (dBcSensor_c*)&adjacent);

    // set pacer final distances
    if (mType == HEIHO_TYPE_PACER) {
        float finalOffset = 16.0 * mDistance - 8.0;
        mFinalPos[0] = mPos.x + finalOffset;
        mFinalPos[1] = mPos.x - finalOffset;
    }

    // set spawning direction
    u8 direction;
    if (mType < HEIHO_TYPE_JUMPER || mType > HEIHO_TYPE_PACER) {
        direction = getPl_LRflag(mPos);
    } else {
        direction = mSpawnDir;
    }
    mDirection = direction;
    mAngle.y = l_heiho_look_angle[direction];

    // set initial state
    switch (mType) {
        case HEIHO_TYPE_WALKER:
        case HEIHO_TYPE_WALKER_LEDGE:
        case HEIHO_TYPE_PACER:
            changeState(StateID_Walk);
            break;
        case HEIHO_TYPE_SLEEPER:
            changeState(StateID_Sleep);
            break;
        case HEIHO_TYPE_JUMPER:
            changeState(StateID_Jump);
            break;
        default:
            changeState(StateID_Idle);
            break;
    }

    return SUCCEEDED;
}

int daEnHeiho_c::execute() {
    // execute state and remove if outisde of zone
    mStateMgr.executeState();
    ActorScrOutCheck(0);
    return true;
}

int daEnHeiho_c::draw() {
    mHeihoModel.entry();
    return SUCCEEDED;
}

void daEnHeiho_c::finalUpdate() {
    // calculate model
    Mtx someMatrix;
    Mtx thirdMatrix;

    // do screen wrapping for levels with it enabled
    dActor_c::changePosAngle(&mPos, &mAngle, 1);

    // set matrix to world position
    PSMTXTrans(mMatrix, mPos.x, mPos.y, mPos.z);

    // apply rotation vector
    mMatrix.YrotM(mAngle.y);
    mMatrix.XrotM(mAngle.x);
    mMatrix.ZrotM(mAngle.z);
    
    // apply center offsets
    PSMTXTrans(someMatrix, 0.0, mCenterOffs.y, 0.0);
    PSMTXConcat(mMatrix, someMatrix, mMatrix);
    PSMTXTrans(thirdMatrix, 0.0, -mCenterOffs.y, 0.0);
    PSMTXConcat(mMatrix, thirdMatrix, mMatrix);

    // set the matrix for the model
    mHeihoModel.setLocalMtx(&mMatrix);
    // see scale using boyon 
    mHeihoModel.setScale(mBoyon.mScale.x, mBoyon.mScale.y, mBoyon.mScale.z);
    mHeihoModel.calc(false);

    return;
}

void daEnHeiho_c::Normal_VsEnHitCheck(dCc_c *cc1, dCc_c *cc2) {
    // collisions with other non-player actors
    if ((mDirection != 1) || (cc1->mCollOffsetX[3] <= 0.0)) {
        if (mDirection != 0) {
            return;
        }
        if (cc1->mCollOffsetX[3] >= 0.0) {
            return;
        }
    }
    if (mStateMgr.getStateID() == &StateID_Walk) {
        changeState(StateID_Turn);
    }
    return;
}

void daEnHeiho_c::Normal_VsPlHitCheck(dCc_c *cc1, dCc_c *cc2) {
    dActor_c *pl = (dActor_c*)cc2->mpOwner;
    // Enfumi_check returns the collision type of the player and enemy 
    int fumi_check = Enfumi_check(cc1, cc2, 0);
    if (fumi_check == 1) { // regular jump
        return reactFumiProc(pl);
    } else if (fumi_check == 3) { // spin jump
        return reactSpinFumiProc(pl);
    } else if (fumi_check == 0) { // hit from the sides
        return dEn_c::Normal_VsPlHitCheck(cc1, cc2);
    }
}

void daEnHeiho_c::Normal_VsYoshiHitCheck(dCc_c *cc1, dCc_c *cc2) {
    dActor_c *pl = (dActor_c*)cc2->mpOwner;
    int fumi_check = Enfumi_check(cc1, cc2, 0);
    if (fumi_check == 1) {
        return reactYoshiFumiProc(pl);
    } else if (fumi_check == 0) {
        return dEn_c::Normal_VsYoshiHitCheck(cc1, cc2);
    }
}

// play the death animation during DieFall
void daEnHeiho_c::initializeState_DieFall() {
    playChrAnim("diefall", m3d::FORWARD_LOOP, 0.0, 1.0);
    return dEn_c::initializeState_DieFall();
}

void daEnHeiho_c::executeState_DieFall() {
    updateModel();
    return dEn_c::executeState_DieFall();
}

// play a falling animation and fall off the stage when stomped
void daEnHeiho_c::initializeState_DieOther() {
    // remove collider
    removeCc();

    // play animation
    playChrAnim("die", m3d::FORWARD_LOOP, 0.0, 1.15);

    mAngle.y = 0;

    mAccelY = -0.1075;
    mSpeed.x = 0.0;
    mSpeed.y = 0.0;
}

void daEnHeiho_c::executeState_DieOther() {
    updateModel();
    calcSpeedY();
    posMove();
}

void daEnHeiho_c::createIceActor() {
    // iceinfo is an array so that __destory_arr can be called
    dIceInfo heihoIceInfo[1] = {
        0,                                      // mFlags
        mVec3_c(mPos.x, mPos.y-3.8, mPos.z),    // mPos
        mVec3_c(1.3, 1.5, 1.5),                 // mScale
        0.0, 
        0.0, 
        0.0, 
        0.0, 
        0.0, 
        0.0, 
        0.0
    };
    mIceMng.createIce(&heihoIceInfo[0], 1);
}

void daEnHeiho_c::setWalkSpeed() {
    mSpeed.x = l_heiho_walk_speed[mDirection];
    return;
}

// play a chr animation
void daEnHeiho_c::playChrAnim(const char* name, m3d::playMode_e playMode, float blendFrame, float rate) {
    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr(name);
    mAnmChr.setAnm(mHeihoModel, resAnmChr, playMode);
    mHeihoModel.setAnm(mAnmChr, blendFrame);
    mAnmChr.setRate(rate);
}

// update animations
void daEnHeiho_c::updateModel() {
    mHeihoModel.play();
    mAnmTexPat.play();
    return;
}

bool daEnHeiho_c::checkForLedge(float xOffset) {
    float xOffs[] = {xOffset, -xOffset};

    mVec3_c tileToCheck;
    tileToCheck.y = 4.0 + mPos.y;
    tileToCheck.z = mPos.z;
    tileToCheck.x = mPos.x + xOffs[mDirection];

    u32 unit = mBc.getUnitKind(tileToCheck.x, mPos.y - 2.0, mLayer);

    if (((unit >> 0x10) & 0xFF) == 8) {
        return false;
    } else {
        float zeroFloat = 0.0;
        bool result = mBc.checkGround(&tileToCheck, &zeroFloat, mLayer, 1, -1);
        if (((!result) || (tileToCheck.y <= zeroFloat)) || (zeroFloat <= mPos.y - 5.0)) {
            return false;
        } else {
            return true;
        }
    }

    return false;
}

// react to being stomped
void daEnHeiho_c::reactFumiProc(dActor_c* player) {
    if (mHealth == 1) {
        mHealth = 0;
        changeState(StateID_Dizzy);
    } else {
        mVec2_c killSpeed = mVec2_c(mSpeed.x, mSpeed.y);
        setDeathInfo_Fumi(player, killSpeed, dEn_c::StateID_DieOther, 0);
    }
}

// react to being spin-stomped
void daEnHeiho_c::reactSpinFumiProc(dActor_c* player) {
    setDeathInfo_SpinFumi(player, 1);
}

// react to yoshi stomp
void daEnHeiho_c::reactYoshiFumiProc(dActor_c* player) {
    setDeathInfo_YoshiFumi(player);
}

void daEnHeiho_c::initializeState_Walk() {
    // play walk animation if coming from turn state
    if (mStateMgr.getOldStateID() != &StateID_Turn) {
        playChrAnim("walk", m3d::FORWARD_LOOP, 0.0, 0.85);
    }
    setWalkSpeed();
}

void daEnHeiho_c::finalizeState_Walk() {}

void daEnHeiho_c::executeState_Walk() {
    // update model & do speed calculations
    updateModel();
    calcSpeedY();
    posMove();
    // finish turning if not facing a direction
    sLib::chaseAngle((short*)&mAngle.y, l_heiho_look_angle[mDirection], 0x800);

    if ((EnBgCheck() & 1) == 0) { // not touching a tile
        // related to walking speed in water? not sure
        if (mBc.isFoot() && (m49 == false) && (mSpeed.y <= 0.0f)) {
            mFootRelated2.x = mFootRelated2.x + m_1eb.x;
        }
    } else { // touching a tile
        mFootRelated2.x = 0.0;
        mSpeed.y = 0.0;
        if (checkForLedge(2.5f) == false) { // check for ledges
            if (mType == HEIHO_TYPE_WALKER_LEDGE) {
                changeState(StateID_Turn);
                return;
            }
        }
    }
    if (mType == HEIHO_TYPE_PACER) { // check if we've reached final pacer distances
        if ((mDirection == 0 && mPos.x >= mFinalPos[0]) || (mDirection == 1 && mPos.x <= mFinalPos[1]))
            changeState(StateID_Turn);
    }
    // turn if touching a wall
    if (mBc.mFlags & 0x15 << mDirection & 0x3f) {
        changeState(StateID_Turn);
    }
    killIfTouchingLava(mPos, 1.0);
    return;
}

void daEnHeiho_c::initializeState_Turn() {
    // quick fix for being stunned while turning
    if (mStateMgr.getOldStateID() == &StateID_Dizzy) {
        playChrAnim("walk", m3d::FORWARD_LOOP, 0.0, 0.85);
    }
    mSpeed.x = 0.0;
    mDirection^=1;
}

void daEnHeiho_c::finalizeState_Turn() {}

// mostly the same as walk state
void daEnHeiho_c::executeState_Turn() {
    updateModel();
    calcSpeedY();
    posMove();

    if ((EnBgCheck() & 1) == 0) {
        if (mBc.isFoot() && (m49 == false) && (mSpeed.y <= 0.0f)) {
            mFootRelated2.x = mFootRelated2.x + m_1eb.x;
        }
    } else {
        mSpeed.y = 0.0;
    }

    killIfTouchingLava(mPos, 1.0);

    // face our new direction, and exit state when finished
    bool doneTurning = sLib::chaseAngle((short*)&mAngle.y, l_heiho_look_angle[mDirection], 0x800);

    if (doneTurning) {
        changeState(StateID_Walk);
    }
    return;
}

void daEnHeiho_c::initializeState_Sleep() {
    playChrAnim("sleep", m3d::FORWARD_LOOP, 0.0, 0.5);
    mAngle.y = 0;
}

void daEnHeiho_c::finalizeState_Sleep() { }

void daEnHeiho_c::executeState_Sleep() {
    updateModel();
    calcSpeedY();
    posMove();

    // handle tile collisions
    if (EnBgCheck() & 1) {
        mSpeed.y = 0.0;
    }

    killIfTouchingLava(mPos, 1.0);
}

void daEnHeiho_c::initializeState_Jump() {
    mJumpCounter = 0;
}

void daEnHeiho_c::finalizeState_Jump() {}

void daEnHeiho_c::executeState_Jump() {
    updateModel();
    calcSpeedY();
    posMove();

    // if touching a ceiling, stop moving upwards
    u8 BgCheck = EnBgCheck();
    if (BgCheck & 2) {
        mSpeed.y = 0.0;
    }

    // touching the grounds
    if (BgCheck & 1) {
        mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
        
        if (mJumpCounter == 3)
            mJumpCounter = 0;

        mJumpCounter++;

        // play animation, set speed, and play sound
        if (mJumpCounter == 3) {
            playChrAnim("jump2", m3d::FORWARD_ONCE, 0.0, 0.6);
            mSpeed.y = 6.0;
            dAudio::g_pSndObjEmy->startSound(SE_PLY_JUMPDAI_HIGH, soundPos, 0);
        } else {
            playChrAnim("jump", m3d::FORWARD_ONCE, 0.0, 0.45);
            mSpeed.y = 4.5;
            dAudio::g_pSndObjEmy->startSound(SE_PLY_JUMPDAI, soundPos, 0);
        }
    }

    killIfTouchingLava(mPos, 1.0);
}

void daEnHeiho_c::initializeState_Dizzy() {
    mRecoverState = (sStateIDIf_c*)mStateMgr.getOldStateID();

    if (mRecoverState == &StateID_Sleep) {
        playChrAnim("dizzy_sleep", m3d::FORWARD_LOOP, 0.0, 1.0);
    } else {
        playChrAnim("dizzy", m3d::FORWARD_LOOP, 0.0, 1.0);
    }

    mSpeed.x = 0;
    mSpeed.y = -4.0;

    mTimer = 0;
}

void daEnHeiho_c::finalizeState_Dizzy() {}

void daEnHeiho_c::executeState_Dizzy() {
    updateModel();
    calcSpeedY();
    posMove();

    if (EnBgCheck() & 1) {
        mSpeed.y = 0.0;
    }

    killIfTouchingLava(mPos, 1.0);

    mVec3_c effectPos(mPos.x, mPos.y + 24.0, 0);
    mVec3_c effectScale(1.0, 1.0, 1.0);
    mDizzyEffect.createEffect("Wm_en_spindamage", 0, &effectPos, nullptr, &effectScale);

    if (mTimer > 600) {
        changeState(*mRecoverState);
        mHealth = 1;
    }
    mTimer++;
}

void daEnHeiho_c::initializeState_Idle() {
    mSpeed.x = 0.0;
}

void daEnHeiho_c::finalizeState_Idle() {}

void daEnHeiho_c::executeState_Idle() {
    updateModel();
    calcSpeedY();
    posMove();

    EnBgCheck();

    killIfTouchingLava(mPos, 1.0);
}