#include <kamek.h>
#include <propelparts/bases/d_a_en_heiho_large.hpp>
#include <propelparts/bases/d_custom_profile.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_actor_manager.hpp>
#include <constants/sound_list.h>

CUSTOM_ACTOR_PROFILE(EN_HEIHO_LARGE, daEnHeihoLarge_c, fProfile::EN_MIDDLE_KURIBO, fProfile::DRAW_ORDER::EN_MIDDLE_KURIBO, 0x12);

STATE_DEFINE(daEnHeihoLarge_c, Walk);
STATE_DEFINE(daEnHeihoLarge_c, Turn);
STATE_DEFINE(daEnHeihoLarge_c, Dizzy);

const char* l_HEIHO_LARGE_res[] = {"heiho", NULL};

const dActorData_c c_HEIHO_LARGE_actor_data = {fProfile::EN_HEIHO_LARGE, 8, -16, 0, 24, 16, 24, 0, 0, 0, 0, 0};
dCustomProfile_c l_HEIHO_LARGE_profile(&g_profile_EN_HEIHO_LARGE, "EN_HEIHO_LARGE", CourseActor::EN_HEIHO_LARGE, &c_HEIHO_LARGE_actor_data, l_HEIHO_LARGE_res);

const dActorData_c c_HEIHO_GIANT_actor_data = {fProfile::EN_HEIHO_GIANT, 8, -16, 0, 36, 27, 39, 0, 0, 0, 0, 0};
dCustomProfile_c l_HEIHO_GIANT_profile(&g_profile_EN_HEIHO_LARGE, "EN_HEIHO_GIANT", CourseActor::EN_HEIHO_GIANT, &c_HEIHO_GIANT_actor_data, l_HEIHO_LARGE_res);

const dActorData_c c_HEIHO_MEGA_actor_data = {fProfile::EN_HEIHO_MEGA, 8, -16, 0, 48, 35, 51, 0, 0, 0, 0, 0};
dCustomProfile_c l_HEIHO_MEGA_profile(&g_profile_EN_HEIHO_LARGE, "EN_HEIHO_MEGA", CourseActor::EN_HEIHO_MEGA, &c_HEIHO_MEGA_actor_data, l_HEIHO_LARGE_res);

const sBcSensorLine l_heiho_large_foot = { 1, -0x8000, 0x8000, 0 };
const sBcSensorLine l_heiho_large_wall = { 1, 0x14000, 0xA000, 0xF000 };

const sBcSensorLine l_heiho_giant_foot = { 1, -0xC000, 0xC000, 0 };
const sBcSensorLine l_heiho_giant_wall = { 1, 0x1F000, 0xA000, 0x17000 };

const sBcSensorLine l_heiho_mega_foot = { 1, -0x10000, 0x10000, 0 };
const sBcSensorLine l_heiho_mega_wall = { 1, 0x24000, 0xA000, 0x1F000 };

int daEnHeihoLarge_c::create() {
    // Setup our model
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);

    mRes = dResMng_c::m_instance->getRes("heiho", "g3d/heiho.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("heiho");
    mModel.create(mdl, &mAllocator, 0x23, 1, nullptr);
    setSoftLight_Enemy(mModel);

    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk");
    mAnmChr.create(mdl, resAnmChr, &mAllocator, 0);
    mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmChr, 0.0f);
    mAnmChr.setRate(0.85f * mWalkSpeed * 2);

    nw4r::g3d::ResAnmTexPat resPat = mRes.GetResAnmTexPat("color");
    mAnmTexPat.create(mdl, resPat, &mAllocator, 0, 1);
    mAnmTexPat.setAnm(mModel, resPat, 0, m3d::FORWARD_ONCE);
    mAnmTexPat.setRate(0.0f, 0);
    mModel.setAnm(mAnmTexPat);

    mAllocator.adjustFrmHeap();

    // Set y acceleration and max speed for gravity
    mAccelY = -0.1875f;
    mSpeedMax.y = -4.0f;

    if (mProfName == fProfile::EN_HEIHO_GIANT) {
        mLargeType = HEIHO_GIANT;

        mScale.set(3.0f, 3.0f, 3.0f);
        mFireHitCount = 3;
        mWalkSpeed = 0.25f;

        mCenterOffs.set(0.0f, 36.0f, 0.0f);
        mVisibleAreaSize.set(54.0f, 78.0f);
        mVisibleAreaOffset.set(0.0f, 36.0f);

        mEatBehaviour = EAT_TYPE_NONE;

        const sCcDatNewF l_heiho_giant_cc = {
            {0.0f, 30.0f},
            {24.0f, 30.0f},
            CC_KIND_ENEMY,
            CC_ATTACK_NONE,
            (BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) | BIT_FLAG(CC_KIND_YOSHI) |
            BIT_FLAG(CC_KIND_ENEMY) | BIT_FLAG(CC_KIND_ITEM) | BIT_FLAG(CC_KIND_TAMA)),
            0xFFBAD94E,
            CC_STATUS_NONE,
            &dEn_c::normal_collcheck,
        };
        mCc.set(this, (sCcDatNewF *)&l_heiho_giant_cc);
        mCc.entry();

        mBc.set(this, l_heiho_giant_foot, nullptr, l_heiho_giant_wall);

        mIceMng.setIceStatus(1, 0, 4);
    } else if (mProfName == fProfile::EN_HEIHO_MEGA) {
        mLargeType = HEIHO_MEGA;

        mScale.set(4.0f, 4.0f, 4.0f);
        mFireHitCount = 1;
        mWalkSpeed = 0.125f;

        mCenterOffs.set(0.0f, 48.0f, 0.0f);
        mVisibleAreaSize.set(70.0f, 102.0f);
        mVisibleAreaOffset.set(0.0f, 48.0f);

        mEatBehaviour = EAT_TYPE_NONE;

        const sCcDatNewF l_heiho_mega_cc = {
            {0.0f, 40.0f},
            {32.0f, 40.0f},
            CC_KIND_ENEMY,
            CC_ATTACK_SHELL,
            BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) | BIT_FLAG(CC_KIND_YOSHI) |
            BIT_FLAG(CC_KIND_ENEMY) | BIT_FLAG(CC_KIND_ITEM) | BIT_FLAG(CC_KIND_TAMA),
            0xFF80900E,
            CC_STATUS_NONE,
            &dEn_c::normal_collcheck,
        };
        mCc.set(this, (sCcDatNewF *)&l_heiho_mega_cc);
        mCc.entry();

        mBc.set(this, l_heiho_mega_foot, nullptr, l_heiho_mega_wall);

        mIceMng.setIceStatus(1, 0, 4);
    } else {
        mLargeType = HEIHO_LARGE;

        mScale.set(2.0f, 2.0f, 2.0f);
        mFireHitCount = 1;
        mWalkSpeed = 0.5f;

        mCenterOffs.set(0.0f, 24.0f, 0.0f);
        mVisibleAreaSize.set(32.0f, 48.0f);
        mVisibleAreaOffset.set(0.0f, 24.0f);

        mEatBehaviour = EAT_TYPE_EAT_PERMANENT;

        const sCcDatNewF l_heiho_large_cc = {
            {0.0f, 20.0f},
            {16.0f, 20.0f},
            CC_KIND_ENEMY,
            CC_ATTACK_NONE,
            BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) | BIT_FLAG(CC_KIND_YOSHI) |
            BIT_FLAG(CC_KIND_ENEMY) | BIT_FLAG(CC_KIND_ITEM) | BIT_FLAG(CC_KIND_TAMA),
            0xFFBAFF7E,
            CC_STATUS_NONE,
            &dEn_c::normal_collcheck,
        };
        mCc.set(this, (sCcDatNewF *)&l_heiho_large_cc);
        mCc.entry();

        mBc.set(this, l_heiho_large_foot, nullptr, l_heiho_large_wall);
    }

    mAmiLayer = mParam >> 16 & 1;
    mColor = mParam >> 24 & 0xF;

    mAnmTexPat.setFrame(mColor, 0);
    
    float zPositions[2] = {1500.0f, -2500.0f};
    mPos.z = zPositions[mAmiLayer];

    // Set spawning direction
    mDirection = getPl_LRflag(mPos);
    mAngle.y = l_base_angleY[mDirection];

    changeState(StateID_Walk);
    
    return SUCCEEDED;
}

int daEnHeihoLarge_c::execute() {
    // Execute state and remove if outside of zone
    mStateMgr.executeState();
    ActorScrOutCheck(SKIP_NONE);
    return true;
}

int daEnHeihoLarge_c::draw() {
    mModel.entry();

    return SUCCEEDED;
}

void daEnHeihoLarge_c::finalUpdate() {
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

    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mBoyoMng.mScale);
    mModel.calc(false);

    return;
}

void daEnHeihoLarge_c::Normal_VsEnHitCheck(dCc_c *self, dCc_c *other) {
    // Collisions with other non-player actors
    if (mLargeType == HEIHO_MEGA) {
        return;
    }
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

void daEnHeihoLarge_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *pl = other->mpOwner;
    // Enfumi_check returns the collision type of the player and enemy 
    bool isStep;
    if (mLargeType != HEIHO_LARGE) {
        isStep = true;
    } else {
        isStep = isState(StateID_Dizzy);
    }
    int fumi_check = Enfumi_check(self, other, isStep);
    if (fumi_check == 0) { // Hit from the sides
        return dEn_c::Normal_VsPlHitCheck(self, other);
    } else { // Jumped on
        return reactFumiProc(pl);
    }
}

void daEnHeihoLarge_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *pl = other->mpOwner;
    bool isStep;
    if (mLargeType != HEIHO_LARGE) {
        isStep = true;
    } else {
        isStep = isState(StateID_Dizzy);
    }
    int fumi_check = Enfumi_check(self, other, 0);
    if (fumi_check == 1) {
        return reactYoshiFumiProc(pl);
    } else if (fumi_check == 0) {
        return dEn_c::Normal_VsYoshiHitCheck(self, other);
    }
}

bool daEnHeihoLarge_c::hitCallback_Shell(dCc_c *self, dCc_c *other) {
    if (mLargeType != HEIHO_LARGE) {
        return true;
    }
    return dEn_c::hitCallback_Shell(self, other);
}

bool daEnHeihoLarge_c::hitCallback_Fire(dCc_c *self, dCc_c *other) {
    if (mLargeType == HEIHO_MEGA) {
        fireballInvalid(self, other);
        return true;
    }
    if (mFireHitCount == 0) {
        mVec3_c center = getCenterPos();
        u8 direction = getTrgToSrcDir_Main(other->mpOwner->getCenterPos().x, getCenterPos().x);
        dActorMng_c::m_instance->createUpCoin(center, direction, 2, 0);
        return dEn_c::hitCallback_Fire(self, other);
    } else {
        ulong sndID;
        if (mLargeType == HEIHO_LARGE) {
            sndID = SE_EMY_KURIBO_M_DAMAGE;
        } else {
            if (mFireHitCount == 3) {
                sndID = SE_EMY_KURIBO_L_DAMAGE_01;
            } else if (mFireHitCount == 2) {
                sndID = SE_EMY_KURIBO_L_DAMAGE_02;
            } else {
                sndID = SE_EMY_KURIBO_L_DAMAGE_03;
            }
        }
        mFireHitCount--;
        boyonBegin();
        nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(mPos);
        dAudio::g_pSndObjEmy->startSound(sndID, soundPos, 0);
    }
    return true;
}

bool daEnHeihoLarge_c::hitCallback_Ice(dCc_c *self, dCc_c *other) {
    if (mLargeType == HEIHO_MEGA) {
        iceballInvalid(self, other);
        return true;
    }
    return dEn_c::hitCallback_Ice(self, other);
}

void daEnHeihoLarge_c::setDeathInfo_Quake(int isMPGP) {
    if (mLargeType == HEIHO_LARGE) {
        return dEn_c::setDeathInfo_Quake(isMPGP);
    }
}

// Play the death animation during DieFall
void daEnHeihoLarge_c::initializeState_DieFall() {
    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("diefall");
    mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmChr, 0.0f);
    mAnmChr.setRate(mWalkSpeed * 2);

    return dEn_c::initializeState_DieFall();
}

void daEnHeihoLarge_c::executeState_DieFall() {
    updateModel();
    return dEn_c::executeState_DieFall();
}

void daEnHeihoLarge_c::setDamage(dActor_c *actor) {
    daPlBase_c *player = (daPlBase_c *)actor;
    if (player->setDamage(this, daPlBase_c::DAMAGE_DEFAULT) && mLargeType == HEIHO_LARGE) {
        u8 direction = getTrgToSrcDir_Main(player->getCenterPos().x, getCenterPos().x);
        if (!isState(StateID_Dizzy)) {
            mDirection = direction;
            mAngle.y = l_base_angleY[mDirection];
        }
        if (isState(StateID_Turn)) {
            changeState(StateID_Walk);
        }
        if (isState(StateID_Walk) || isState(StateID_Turn)) {
            setWalkSpeed();
        }
    }
}

bool daEnHeihoLarge_c::createIceActor() {
    // Iceinfo is an array so that __destory_arr can be called
    mVec3_c baseIceScale(1.2f, 1.3f, 1.5f);
    dIceInfo heihoIceInfo[1] = {
        0x1000,                                  // mFlags
        mVec3_c(mPos.x, mPos.y-4.0f, mPos.z),    // mPos
        baseIceScale * mScale.x,                 // mScale
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f
    };
    return mIceMng.createIce(&heihoIceInfo[0], 1);
}

void daEnHeihoLarge_c::FumiJumpSet(dActor_c *actor) {
    float jumpSpeedAdd = 0.2815f;
    if (mLargeType == HEIHO_MEGA) {
        jumpSpeedAdd = 1.372f;
    }
    PlayerFumiJump(actor, dAcPy_c::msc_JUMP_SPEED + jumpSpeedAdd);
}

void daEnHeihoLarge_c::FumiScoreSet(dActor_c *actor) {
    if (isState(StateID_Dizzy) || mLargeType != HEIHO_LARGE) {
        return;
    }
    return dEn_c::FumiScoreSet(actor);
}

void daEnHeihoLarge_c::setWalkSpeed() {
    static const float dirSpeed[] = { mWalkSpeed, -mWalkSpeed };
    mSpeed.x = dirSpeed[mDirection];
}

void daEnHeihoLarge_c::updateModel() {
    mModel.play();
}

// React to being stomped
void daEnHeihoLarge_c::reactFumiProc(dActor_c* player) {
    if (!isState(StateID_Dizzy) && mLargeType == HEIHO_LARGE) {
        changeState(StateID_Dizzy);
    } else {
        boyonBegin();
    }
}

// React to yoshi stomp
void daEnHeihoLarge_c::reactYoshiFumiProc(dActor_c* player) {
    if (mLargeType == HEIHO_LARGE) {
        setDeathInfo_YoshiFumi(player);
    } else {
        boyonBegin();
    }
}

void daEnHeihoLarge_c::initializeState_Walk() {
    if (mStateMgr.getOldStateID() != &StateID_Turn) {
        nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk");
        mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
        mModel.setAnm(mAnmChr, 0.0f);
        mAnmChr.setRate(0.85f * mWalkSpeed * 2);
    }
    setWalkSpeed();
}

void daEnHeihoLarge_c::finalizeState_Walk() {}

void daEnHeihoLarge_c::executeState_Walk() {
    // Update model & do speed calculations
    updateModel();
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
    }
    // Turn if touching a wall
    if (mBc.mFlags & 0x15 << mDirection & 0x3f) {
        changeState(StateID_Turn);
    }
    if (mLargeType == HEIHO_LARGE) {
        WaterCheck(mPos, 1.0f);
    }
    return;
}

void daEnHeihoLarge_c::initializeState_Turn() {
    mSpeed.x = 0.0f;
    mDirection^=1;
}

void daEnHeihoLarge_c::finalizeState_Turn() {}

// Mostly the same as walk state
void daEnHeihoLarge_c::executeState_Turn() {
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

    if (mLargeType == HEIHO_LARGE) {
        WaterCheck(mPos, 1.0f);
    }

    // Face our new direction, and exit state when finished
    bool doneTurning = sLib::chaseAngle((short*)&mAngle.y, l_base_angleY[mDirection], 0x800);

    if (doneTurning) {
        changeState(StateID_Walk);
    }
    return;
}

void daEnHeihoLarge_c::initializeState_Dizzy() {
    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("dizzy");
    mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmChr, 0.0f);
    mAnmChr.setRate(mWalkSpeed * 2);

    mSpeed.x = 0.0f;
    mSpeed.y = -4.0f;

    mTimer = 0;
}

void daEnHeihoLarge_c::finalizeState_Dizzy() {}

void daEnHeihoLarge_c::executeState_Dizzy() {
    updateModel();
    calcSpeedY();
    posMove();

    if (EnBgCheck() & 1) {
        mSpeed.y = 0.0f;
    }

    if (mLargeType == HEIHO_LARGE) {
        WaterCheck(mPos, 1.0f);
    }

    mVec3_c effectPos(mPos.x, mPos.y + mCenterOffs.y * 2, 0.0f);
    mDizzyEffect.createEffect("Wm_en_spindamage", 0, &effectPos, nullptr, &mScale);

    if (mTimer > 600) {
        changeState(StateID_Walk);
    }
    mTimer++;
}