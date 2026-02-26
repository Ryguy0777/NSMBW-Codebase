#include <kamek.h>
#include <propelparts/bases/d_a_en_puchi_pakkun.hpp>
#include <propelparts/bases/d_custom_profile.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/mLib/m_effect.hpp>
#include <constants/sound_list.h>
#include <game/bases/d_a_freezer.hpp>

CUSTOM_ACTOR_PROFILE(EN_PUCHI_PAKKUN, daEnPuchiPakkun_c, fProfile::EN_BLACK_PAKKUN, fProfile::DRAW_ORDER::EN_BLACK_PAKKUN, 0x12);

STATE_DEFINE(daEnPuchiPakkun_c, Idle);
STATE_DEFINE(daEnPuchiPakkun_c, Walk);
STATE_DEFINE(daEnPuchiPakkun_c, Turn);
STATE_DEFINE(daEnPuchiPakkun_c, Jump);
STATE_DEFINE(daEnPuchiPakkun_c, FireSpit);
STATE_DEFINE(daEnPuchiPakkun_c, IceWait);

const char* l_PUCHI_PAKKUN_res[] = {"pakkun_puchi", NULL};
const dActorData_c c_PUCHI_PAKKUN_actor_data = {fProfile::EN_PUCHI_PAKKUN, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c l_PUCHI_PAKKUN_profile(&g_profile_EN_PUCHI_PAKKUN, "EN_PUCHI_PAKKUN", CourseActor::EN_PUCHI_PAKKUN, &c_PUCHI_PAKKUN_actor_data, l_PUCHI_PAKKUN_res);

const s16 l_nipper_angleY[] = { 0x4000, -0x4000 };
const s16 l_nipper_turn_angleY[] = { 0x3999, -0x3999 };

const float daEnPuchiPakkun_c::smc_WALK_SPEED = 0.5f;

const sBcSensorPoint l_nipper_head = { 0, 0x0, 0x10000 };
const sBcSensorLine l_nipper_foot = { 1, -0x4000, 0x4000, 0 };
const sBcSensorLine l_nipper_wall = { 1, 0x3000, 0x8000, 0x8000 };

const sCcDatNewF l_nipper_cc = {
    {0.0f, 8.0f},
    {8.0f, 8.0f},
    CC_KIND_ENEMY,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) | BIT_FLAG(CC_KIND_YOSHI) |
    BIT_FLAG(CC_KIND_ENEMY) | BIT_FLAG(CC_KIND_TAMA),
    0xFFBAD35E,  
    CC_STATUS_NONE,
    &dEn_c::normal_collcheck,
};

int daEnPuchiPakkun_c::create() {
    // Setup our model
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);
    mRes = dResMng_c::m_instance->getRes("pakkun_puchi", "g3d/pakkun_puchi.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("pakkun_puchi");
    mNipperModel.create(mdl, &mAllocator, 0x20, 1, nullptr);
    setSoftLight_Enemy(mNipperModel);

    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("attack");
    mAnmChr.create(mdl, resAnmChr, &mAllocator, 0);
    playChrAnim("attack", m3d::FORWARD_LOOP, 0.0f, 1.0f);

    mAllocator.adjustFrmHeap();

    // Set y acceleration and max speed for gravity
    mAccelY = -0.1875f;
    mSpeedMax.y = -4.0f;

    // Register the cc data (hitbox)
    mCc.set(this, (sCcDatNewF *)&l_nipper_cc);
    mCc.entry();

    mDirection = getPl_LRflag(mPos);
    mAngle.y = l_nipper_angleY[mDirection];

    // Assign settings variables
    mWalks = (mParam >> 17) & 1;
    mJumpHeight = (mParam >> 12) & 0xF;
    mSpitsFire = (mParam >> 1) & 1;

    // mCenterOffs is used to set the "center" of the actor
    // For yoshi tongue and dieFall
    mCenterOffs.set(0.0f, 8.0f, 0.0f);

    // Set size for model culling
    mVisibleAreaSize.set(16.0f, 16.0f);
    mVisibleAreaOffset.set(0.0f, 8.0f);

    // Set yoshi eating behavior
    if (mSpitsFire) {
        mEatBehaviour = EAT_TYPE_FIREBALL;
    } else {
        mEatBehaviour = EAT_TYPE_EAT_PERMANENT;
    }

    // Tile sensors
    mBc.set(this, l_nipper_foot, l_nipper_head, l_nipper_wall);

    if (mParam & 1) { // Spawn frozen
        changeState(StateID_IceWait);
        // Create an AC_FREEZER at our position and layer
        construct(fProfile::AC_FREEZER, this, 0, &mPos, nullptr, mLayer);
        mCc.release();
    } else {
        if (mWalks) {
            changeState(StateID_Walk);
        } else {
            changeState(StateID_Idle);
        }
    }

    return SUCCEEDED;
}

int daEnPuchiPakkun_c::execute() {
    // Execute state and remove if outside of zone
    mStateMgr.executeState();
    ActorScrOutCheck(SKIP_NONE);
    return true;
}

int daEnPuchiPakkun_c::draw() {
    mNipperModel.entry();
    return SUCCEEDED;
}

void daEnPuchiPakkun_c::finalUpdate() {
    // Calculate model matricies
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;

    // Do screen wrapping for levels with it enabled
    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.YrotM(angle.y);

    mMatrix.concat(mMtx_c::createTrans(0.0f, mCenterOffs.y, 0.0f));
    mMatrix.XrotM(angle.x);
    mMatrix.concat(mMtx_c::createTrans(0.0f, -mCenterOffs.y, 0.0f));

    mMatrix.concat(mMtx_c::createTrans(0.0f, mCenterOffs.y * 2, 0.0f));
    mMatrix.ZrotM(angle.z);
    mMatrix.concat(mMtx_c::createTrans(0.0f, -mCenterOffs.y * 2, 0.0f));

    mNipperModel.setLocalMtx(&mMatrix);
    mNipperModel.setScale(mBoyoMng.mScale);
    mNipperModel.calc(false);

    return;
}

void daEnPuchiPakkun_c::Normal_VsEnHitCheck(dCc_c *self, dCc_c *other) {
    // Collisions with other non-player actors
    if ((mDirection != 1) || (self->mCollOffsetX[3] <= 0.0f)) {
        if (mDirection != 0) {
            return;
        }
        if (self->mCollOffsetX[3] >= 0.0f) {
            return;
        }
    }
    if (mStateMgr.getStateID() == &StateID_Walk) {
        changeState(StateID_Turn);
    }
    return;
}

void daEnPuchiPakkun_c::initializeState_Ice() {
    if (!mWalks) {
        // Munchers are slightly smaller when encased in ice, so we replicate that here
        mScale.set(0.89f, 0.89f, 0.89f);
        // Store x pos
        mStoredIcePos = mPos.x;
        // Set animation to open mouth
        mAnmChr.setFrame(0.0f);
    }
    return dEn_c::initializeState_Ice();
}

void daEnPuchiPakkun_c::finalizeState_Ice() {
    // Restore everything
    if (!mWalks) {
        mScale.set(1.0f, 1.0f, 1.0f);
        mPos.x = mStoredIcePos;
        playChrAnim("attack", m3d::FORWARD_LOOP, 0.0f, 1.0f);
    }
    return dEn_c::finalizeState_Ice();
}

bool daEnPuchiPakkun_c::createIceActor() {
    // Iceinfo is an array so that __destory_arr can be called
    dIceInfo nipperIceInfo[1] = {
        0x1000,                                 // mFlags
        mVec3_c(mPos.x, mPos.y, mPos.z+5.0f),   // mPos
        mVec3_c(0.8f, 0.8f, 0.8f),              // mScale
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f
    };

    if (mWalks) {
        nipperIceInfo[0].mFlags = 0;
        nipperIceInfo[0].mPos.set(mPos.x, mPos.y-1.0f, mPos.z);
        nipperIceInfo[0].mScale.set(1.0f, 1.0f, 1.0f);
    }

    return mIceMng.createIce(&nipperIceInfo[0], 1);
}

void daEnPuchiPakkun_c::playChrAnim(const char* name, m3d::playMode_e playMode, float blendFrame, float rate) {
	nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr(name);
	mAnmChr.setAnm(mNipperModel, resAnmChr, playMode);
	mNipperModel.setAnm(mAnmChr, blendFrame);
	mAnmChr.setRate(rate);

    // Sync our animations with other nippers/munchers
    float frame = dScStage_c::m_exeFrame - (dScStage_c::m_exeFrame / int(mAnmChr.mFrameMax)) * int(mAnmChr.mFrameMax);
    mAnmChr.setFrame(frame);
}

void daEnPuchiPakkun_c::updateModel() {
    mNipperModel.play();
    return;
}

void daEnPuchiPakkun_c::setWalkSpeed() {
    static const float dirSpeed[] = { smc_WALK_SPEED, -smc_WALK_SPEED };
    mSpeed.x = dirSpeed[mDirection];
    return;
}

bool daEnPuchiPakkun_c::checkForLedge(float xOffset) {
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

bool daEnPuchiPakkun_c::isPlayerAbove() {
    for (int i = 0; i < PLAYER_COUNT; i++) {
        dAcPy_c *player = daPyMng_c::getPlayer(i);
        // Grab any active players
        if (player) {
            // Are we in the same x range as the nipper?
            if (10.0f >= std::fabs(player->mPos.x - mPos.x)) {
                // Are we in the y range?
                if (mPos.y + 8.0f <= player->mPos.y && player->mPos.y <= mPos.y + 104.0f) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool daEnPuchiPakkun_c::isPlayerInFireRange() {
    for (int i = 0; i < PLAYER_COUNT; i++) {
        dAcPy_c *player = daPyMng_c::getPlayer(i);
        if (player) {
            if ((88.0f >= std::fabs(player->mPos.x - mPos.x)) && (48.0f >= abs(player->mPos.y - mPos.y))) {
                mFireDirection = getTrgToSrcDir_Main(player->mPos.x + player->mCenterOffs.x, mPos.x + mCenterOffs.x);
                setFireDistance(abs(player->mPos.x - mPos.x));
                return true;
            }
        }
    }
    return false;
}

void daEnPuchiPakkun_c::setFireDistance(float distance) {
    if (distance >= 65.0f) {
        mFireDist = 3;
    } else if (distance >= 49.0f) {
        mFireDist = 2;
    } else if (distance >= 30.0f) {
        mFireDist = 1;
    } else {
        mFireDist = 0;
    }
}

void daEnPuchiPakkun_c::initializeState_Idle() {
    playChrAnim("attack", m3d::FORWARD_LOOP, 0.0f, 1.0f);
}

void daEnPuchiPakkun_c::finalizeState_Idle() {}

void daEnPuchiPakkun_c::executeState_Idle() {
    updateModel();
    calcSpeedY();
    posMove();

    if ((EnBgCheck()) & 1) {
        if (dAudio::isBgmAccentSign(1)) {
            mIsBahJump = true;
            changeState(StateID_Jump);
        }
        if (isPlayerAbove()) {
            changeState(StateID_Jump);
        }
        if (mSpitsFire && isPlayerInFireRange() && mFireCooldown == 0) {
            changeState(StateID_FireSpit);
        }
    }
    if (mFireCooldown > 0) {
        mFireCooldown--;
    }

    WaterCheck(mPos, 1.0f);
}

void daEnPuchiPakkun_c::initializeState_Walk() {
    if (mStateMgr.getOldStateID() != &StateID_Turn) {
        playChrAnim("jump", m3d::FORWARD_LOOP, 0.0f, 1.0f);
    }
    setWalkSpeed();
}

void daEnPuchiPakkun_c::finalizeState_Walk() {}

void daEnPuchiPakkun_c::executeState_Walk() {
    updateModel();
    calcSpeedY();
    posMove();
    sLib::chaseAngle((short*)&mAngle.y, l_nipper_angleY[mDirection], 0x800);

    if (!EnBgCheck() & 1) {
        if (mBc.isFoot() && (mInLiquid == false) && (mSpeed.y <= 0.0f)) {
            mFootPush2.x = mFootPush2.x + m_1eb.x;
        }
    } else {
        if (checkForLedge(4.0f) == false) {
            changeState(StateID_Turn);
            return;
        }
        if (mBc.mFlags & 0x15 << mDirection & 0x3f) {
            mSpeed.x = 0.0f;
            if (EnBgCheck() & 1) {
                changeState(StateID_Turn);
            }
        } else {
            mFootPush2.x = 0.0f;
            mSpeed.y = 1.2;
        }
    }
    if (isPlayerAbove()) {
        changeState(StateID_Jump);
    }
    if (dAudio::isBgmAccentSign(1)) {
        mIsBahJump = true;
        changeState(StateID_Jump);
    }
    WaterCheck(mPos, 1.0f);
}

void daEnPuchiPakkun_c::initializeState_Turn() {
    if (mStateMgr.getOldStateID() != &StateID_Walk) {
        playChrAnim("jump", m3d::FORWARD_LOOP, 0.0f, 1.0f);
    } else {
        mDirection ^= 1;
    }
    mSpeed.x = 0.0f;
}

void daEnPuchiPakkun_c::finalizeState_Turn() {}

void daEnPuchiPakkun_c::executeState_Turn() {
    updateModel();
    calcSpeedY();
    posMove();

    if ((EnBgCheck() & 1) == 0) {
        if (mBc.isFoot() && (mInLiquid == false) && (mSpeed.y <= 0.0f)) {
            mFootPush2.x = mFootPush2.x + m_1eb.x;
        }
    } else {
        mSpeed.y = 0.0f;
    }

    if (dAudio::isBgmAccentSign(1)) {
        mIsBahJump = true;
        changeState(StateID_Jump);
    }

    WaterCheck(mPos, 1.0f);

    // Face our new direction, and exit state when finished
    bool doneTurning = sLib::chaseAngle((short*)&mAngle.y, l_nipper_turn_angleY[mDirection], 0x800);

    if (doneTurning) {
        changeState(StateID_Walk);
    }
    return;
}

void daEnPuchiPakkun_c::initializeState_Jump() {
    playChrAnim("jump", m3d::FORWARD_LOOP, 0.0f, 1.0f);

    mSpeed.x = 0.0f;
    if (mIsBahJump) {
        mSpeed.y = 2.0f;
    } else {
        switch (mJumpHeight) {
            default:
                mSpeed.y = 5.5f; 
                break;
            case 1:
                mSpeed.y = 4.5f;
                break;
            case 2:
                mSpeed.y = 3.5f;
                break;
        }
    }
}

void daEnPuchiPakkun_c::finalizeState_Jump() {
    mIsBahJump = false;
}

void daEnPuchiPakkun_c::executeState_Jump() {
    updateModel();
    calcSpeedY();
    posMove();

    if ((EnBgCheck()) & 1) {
        if (!mIsBahJump) {
            mEf::createEffect("Wm_en_landsmoke_s", 0, &mPos, (mAng3_c*)nullptr, (mVec3_c*)nullptr);
        }
        if (mWalks) {
            changeState(StateID_Walk);
        } else {
            changeState(StateID_Idle);
        }
    }

    WaterCheck(mPos, 1.0f);
}

void daEnPuchiPakkun_c::initializeState_IceWait() {
    mScale.set(0.89f, 0.89f, 0.89f);
    mAnmChr.setFrame(0.0f);
}

void daEnPuchiPakkun_c::finalizeState_IceWait() {}

void daEnPuchiPakkun_c::executeState_IceWait() {
    u8 fireballState = 0;
    daFreezer_c *freezer;
    // Make sure our AC_FREEZER still exists
    for (freezer = (daFreezer_c *)getConnectChild(); freezer != (daFreezer_c *)nullptr; freezer = (daFreezer_c *)freezer->getConnectBrNext()) {
        if (freezer != (daFreezer_c *)nullptr) {
            // If it does, check if it's melting
            fireballState = (freezer->mFireBallState != 0) + 1;
        }
    }
    // We're MELTING
    if (fireballState == 2) {
        mScale.set(1.0f, 1.0f, 1.0f);
    }
    // Our AC_FREEZER no longer exists, so we've melted
    if (fireballState == 0) {
        mCc.entry();
        if (mWalks) {
            changeState(StateID_Walk);
        } else {
            changeState(StateID_Idle);
        }
    }
    
}

void daEnPuchiPakkun_c::initializeState_FireSpit() {
    playChrAnim("spit", m3d::FORWARD_LOOP, 0.0f, 1.0f);
    mFireTimer = 15;
}

void daEnPuchiPakkun_c::finalizeState_FireSpit() {
    mFireTimer = 0;
    mSpatFireCount = 0;
    mFireCooldown = 45;
}

void daEnPuchiPakkun_c::executeState_FireSpit() {
    updateModel();
    calcSpeedY();
    posMove();
    EnBgCheck();

    if (mFireTimer == 15 && isPlayerInFireRange()) {
        mVec3_c firePos(mPos.x, mPos.y + 10.0f, mPos.z);
        construct(fProfile::AC_PAKKUN_PUCHI_FIRE, mFireDist << 4 | mFireDirection, &firePos, &mAngle, mLayer);
        mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
        dAudio::g_pSndObjEmy->startSound(SE_EMY_FIRE_BROS_FIRE, soundPos, 0);
        mSpatFireCount++;
        mFireTimer = 0;
    }
    if (mSpatFireCount > 3 || !isPlayerInFireRange()) {
        changeState(StateID_Idle);
    }
    mFireTimer++;
}