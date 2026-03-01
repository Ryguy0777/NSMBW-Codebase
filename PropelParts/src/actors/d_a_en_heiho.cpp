#include <kamek.h>
#include <propelparts/bases/d_a_en_heiho.hpp>
#include <propelparts/bases/d_custom_profile.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_audio.hpp>
#include <constants/sound_list.h>

CUSTOM_ACTOR_PROFILE(EN_HEIHO, daEnHeiho_c, fProfile::EN_KURIBO, fProfile::DRAW_ORDER::EN_KURIBO, 0x12);

STATE_DEFINE(daEnHeiho_c, Walk);
STATE_DEFINE(daEnHeiho_c, Turn);
STATE_DEFINE(daEnHeiho_c, Sleep);
STATE_DEFINE(daEnHeiho_c, Jump);
STATE_DEFINE(daEnHeiho_c, Dizzy);
STATE_DEFINE(daEnHeiho_c, Idle);

const char* l_HEIHO_res[] = {"heiho", NULL};
const dActorData_c c_HEIHO_actor_data = {fProfile::EN_HEIHO, 8, -16, 0, 12, 8, 12, 0, 0, 0, 0, 0};
dCustomProfile_c l_HEIHO_profile(&g_profile_EN_HEIHO, "EN_HEIHO", CourseActor::EN_HEIHO, &c_HEIHO_actor_data, l_HEIHO_res);

const float daEnHeiho_c::smc_WALK_SPEED = 0.6f;

const sBcSensorPoint l_heiho_head = { 0, 0x0, 0x14000 };
const sBcSensorLine l_heiho_foot = { 1, -0x4000, 0x4000, 0 };
const sBcSensorLine l_heiho_wall = { 1, 0x3000, 0x8000, 0x8000 };

const sCcDatNewF l_heiho_cc = {
    {0.0f, 10.0f},
    {8.0f, 10.0f},
    CC_KIND_ENEMY,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) | BIT_FLAG(CC_KIND_YOSHI) |
    BIT_FLAG(CC_KIND_ENEMY) | BIT_FLAG(CC_KIND_ITEM) | BIT_FLAG(CC_KIND_TAMA),
    0xFFBAFFFE,
    CC_STATUS_NONE,
    &dEn_c::normal_collcheck,
};

int daEnHeiho_c::create() {
    // Setup our model
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);
    
    loadModel();

    mAllocator.adjustFrmHeap();

    // Set y acceleration and max speed for gravity
    mAccelY = -0.1875f;
    mSpeedMax.y = -4.0f;

    // Register the cc data (hitbox)
    mCc.set(this, (sCcDatNewF *)&l_heiho_cc);
    mCc.entry();

    // Assign settings variables
    mAmiLayer = mParam >> 16 & 1;

    mType = (HEIHO_TYPE_e)(mParam >> 28 & 0xF);
    mColor = mParam >> 24 & 0xF;
    mHealth = mParam >> 17 & 1;
    mDistance = mParam >> 12 & 0xF;
    mSpawnDir = (mParam >> 8 & 1)^1;

    mAnmTexPat.setFrame(mColor, 0);

    // mCenterOffs is used to set the "center" of the actor
    // For yoshi tongue and dieFall
    mCenterOffs.set(0.0f, 12.0f, 0.0f);

    // Set size for model culling
    mVisibleAreaSize.set(16.0f, 24.0f);
    mVisibleAreaOffset.set(0.0f, 12.0f);

    // Set yoshi eating behavior
    mEatBehaviour = EAT_TYPE_EAT_PERMANENT;

    float zPositions[2] = {1500.0f, -2500.0f};
    mPos.z = zPositions[mAmiLayer];

    // Tile sensors
    mBc.set(this, l_heiho_foot, l_heiho_head, l_heiho_wall);

    // Set pacer final distances
    if (mType == HEIHO_TYPE_PACER) {
        float finalOffset = 16.0f * mDistance - 8.0f;
        mFinalPos[0] = mPos.x + finalOffset;
        mFinalPos[1] = mPos.x - finalOffset;
    }

    // Set spawning direction
    u8 direction;
    if (mType < HEIHO_TYPE_JUMPER || mType > HEIHO_TYPE_PACER) {
        direction = getPl_LRflag(mPos);
    } else {
        direction = mSpawnDir;
    }
    mDirection = direction;
    mAngle.y = l_base_angleY[direction];

    setInitialState();

    return SUCCEEDED;
}

int daEnHeiho_c::execute() {
    // Execute state and remove if outside of zone
    mStateMgr.executeState();
    ActorScrOutCheck(SKIP_NONE);
    return true;
}

int daEnHeiho_c::draw() {
    drawModel();
    return SUCCEEDED;
}

void daEnHeiho_c::finalUpdate() {
    calcModel();

    return;
}

void daEnHeiho_c::Normal_VsEnHitCheck(dCc_c *self, dCc_c *other) {
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

void daEnHeiho_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *pl = (dActor_c*)other->mpOwner;
    // Enfumi_check returns the collision type of the player and enemy 
    int fumi_check = Enfumi_check(self, other, 0);
    if (fumi_check == 1) { // Regular jump
        return reactFumiProc(pl);
    } else if (fumi_check == 3) { // Spin jump
        return reactSpinFumiProc(pl);
    } else if (fumi_check == 0) { // Hit from the sides
        return dEn_c::Normal_VsPlHitCheck(self, other);
    }
}

void daEnHeiho_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *pl = (dActor_c*)other->mpOwner;
    int fumi_check = Enfumi_check(self, other, 0);
    if (fumi_check == 1) {
        return reactYoshiFumiProc(pl);
    } else if (fumi_check == 0) {
        return dEn_c::Normal_VsYoshiHitCheck(self, other);
    }
}

// Play the death animation during DieFall
void daEnHeiho_c::initializeState_DieFall() {
    playChrAnim("diefall", m3d::FORWARD_LOOP, 0.0f, 1.0f);
    return dEn_c::initializeState_DieFall();
}

void daEnHeiho_c::executeState_DieFall() {
    updateModel();
    return dEn_c::executeState_DieFall();
}

// Play a falling animation and fall off the stage when stomped
void daEnHeiho_c::initializeState_DieOther() {
    // Remove collider
    removeCc();

    // Play animation
    playChrAnim("die", m3d::FORWARD_LOOP, 0.0f, 1.15f);

    mAngle.y = 0;

    mAccelY = -0.1075f;
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
}

void daEnHeiho_c::executeState_DieOther() {
    updateModel();
    calcSpeedY();
    posMove();
}

void daEnHeiho_c::setDamage(dActor_c *actor) {
    daPlBase_c *pl = (daPlBase_c *) actor;
    if (pl->setDamage(this, daPlBase_c::DAMAGE_DEFAULT)) {
        setTurnByPlayerHit(actor);
    }
}

bool daEnHeiho_c::createIceActor() {
    // Iceinfo is an array so that __destory_arr can be called
    dIceInfo heihoIceInfo[1] = {
        0,                                       // mFlags
        mVec3_c(mPos.x, mPos.y-3.8f, mPos.z),    // mPos
        mVec3_c(1.3f, 1.5f, 1.5f),               // mScale
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

// Load model
void daEnHeiho_c::loadModel() {
    mRes = dResMng_c::m_instance->getRes("heiho", "g3d/heiho.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("heiho");
    mHeihoModel.create(mdl, &mAllocator, 0x23, 1, nullptr);
    setSoftLight_Enemy(mHeihoModel);

    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("idle");
    mAnmChr.create(mdl, resAnmChr, &mAllocator, 0);
    playChrAnim("idle", m3d::FORWARD_LOOP, 0.0f, 2.0f);

    nw4r::g3d::ResAnmTexPat resPat = mRes.GetResAnmTexPat("color");
    mAnmTexPat.create(mdl, resPat, &mAllocator, 0, 1);
    mAnmTexPat.setAnm(mHeihoModel, resPat, 0, m3d::FORWARD_ONCE);
    mAnmTexPat.setRate(0.0f, 0);
    mHeihoModel.setAnm(mAnmTexPat);
}

// Update animations
void daEnHeiho_c::updateModel() {
    mHeihoModel.play();
}

// Draw model
void daEnHeiho_c::drawModel() {
    mHeihoModel.entry();
}

void daEnHeiho_c::calcModel() {
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

    mHeihoModel.setLocalMtx(&mMatrix);
    mHeihoModel.setScale(mBoyoMng.mScale);
    mHeihoModel.calc(false);
}

void daEnHeiho_c::setTurnByPlayerHit(dActor_c *player) {
    u8 direction = getTrgToSrcDir_Main(player->getCenterPos().x, getCenterPos().x);
    if (!isState(StateID_Sleep) && !isState(StateID_Dizzy)) {
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

void daEnHeiho_c::setInitialState() {
    // Set initial state
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
}

void daEnHeiho_c::setWalkSpeed() {
    static const float dirSpeed[] = { smc_WALK_SPEED, -smc_WALK_SPEED };
    mSpeed.x = dirSpeed[mDirection];
}

// Play a chr animation
void daEnHeiho_c::playChrAnim(const char* name, m3d::playMode_e playMode, float blendFrame, float rate) {
    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr(name);
    mAnmChr.setAnm(mHeihoModel, resAnmChr, playMode);
    mHeihoModel.setAnm(mAnmChr, blendFrame);
    mAnmChr.setRate(rate);
}

bool daEnHeiho_c::checkForLedge(float xOffset) {
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

// React to being stomped
void daEnHeiho_c::reactFumiProc(dActor_c* player) {
    if (mHealth == 1) {
        mHealth = 0;
        changeState(StateID_Dizzy);
    } else {
        mVec2_c killSpeed(mSpeed.x, mSpeed.y);
        setDeathInfo_Fumi(player, killSpeed, dEn_c::StateID_DieOther, 0);
    }
}

// React to being spin-stomped
void daEnHeiho_c::reactSpinFumiProc(dActor_c* player) {
    setDeathInfo_SpinFumi(player, 1);
}

// React to yoshi stomp
void daEnHeiho_c::reactYoshiFumiProc(dActor_c* player) {
    setDeathInfo_YoshiFumi(player);
}

void daEnHeiho_c::initializeState_Walk() {
    // Play walk animation if coming from turn state
    if (mStateMgr.getOldStateID() != &StateID_Turn) {
        playChrAnim("walk", m3d::FORWARD_LOOP, 0.0f, 0.85f);
    }
    setWalkSpeed();
}

void daEnHeiho_c::finalizeState_Walk() {}

void daEnHeiho_c::executeState_Walk() {
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
        if (checkForLedge(2.5f) == false) { // Check for ledges
            if (mType == HEIHO_TYPE_WALKER_LEDGE) {
                changeState(StateID_Turn);
                return;
            }
        }
    }
    if (mType == HEIHO_TYPE_PACER) { // Check if we've reached final pacer distances
        if ((mDirection == 0 && mPos.x >= mFinalPos[0]) || (mDirection == 1 && mPos.x <= mFinalPos[1]))
            changeState(StateID_Turn);
    }
    // Turn if touching a wall
    if (mBc.mFlags & 0x15 << mDirection & 0x3f) {
        changeState(StateID_Turn);
    }
    WaterCheck(mPos, 1.0f);
    return;
}

void daEnHeiho_c::initializeState_Turn() {
    // Quick fix for being stunned while turning
    if (mStateMgr.getOldStateID() == &StateID_Dizzy) {
        playChrAnim("walk", m3d::FORWARD_LOOP, 0.0f, 0.85f);
    }
    mSpeed.x = 0.0f;
    mDirection^=1;
}

void daEnHeiho_c::finalizeState_Turn() {}

// Mostly the same as walk state
void daEnHeiho_c::executeState_Turn() {
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

    WaterCheck(mPos, 1.0f);

    // Face our new direction, and exit state when finished
    bool doneTurning = sLib::chaseAngle((short*)&mAngle.y, l_base_angleY[mDirection], 0x800);

    if (doneTurning) {
        changeState(StateID_Walk);
    }
    return;
}

void daEnHeiho_c::initializeState_Sleep() {
    playChrAnim("sleep", m3d::FORWARD_LOOP, 0.0f, 1.0f);
    mAngle.y = 0;
}

void daEnHeiho_c::finalizeState_Sleep() { }

void daEnHeiho_c::executeState_Sleep() {
    updateModel();
    calcSpeedY();
    posMove();

    // Handle tile collisions
    if (EnBgCheck() & 1) {
        mSpeed.y = 0.0f;
    }

    WaterCheck(mPos, 1.0f);
}

void daEnHeiho_c::initializeState_Jump() {
    mJumpCounter = 0;
}

void daEnHeiho_c::finalizeState_Jump() {}

void daEnHeiho_c::executeState_Jump() {
    updateModel();
    calcSpeedY();
    posMove();

    // If touching a ceiling, stop moving upwards
    u8 BgCheck = EnBgCheck();
    if (BgCheck & 2) {
        mSpeed.y = 0.0f;
    }

    // Touching the ground
    if (BgCheck & 1) {
        mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
        
        if (mJumpCounter == 3)
            mJumpCounter = 0;

        mJumpCounter++;

        // Play animation, set speed, and play sound
        if (mJumpCounter == 3) {
            playChrAnim("jump2", m3d::FORWARD_ONCE, 0.0f, 0.6f);
            mSpeed.y = 6.0f;
            dAudio::g_pSndObjEmy->startSound(SE_PLY_JUMPDAI_HIGH, soundPos, 0);
        } else {
            playChrAnim("jump", m3d::FORWARD_ONCE, 0.0f, 0.45f);
            mSpeed.y = 4.5f;
            dAudio::g_pSndObjEmy->startSound(SE_PLY_JUMPDAI, soundPos, 0);
        }
    }

    WaterCheck(mPos, 1.0);
}

void daEnHeiho_c::initializeState_Dizzy() {
    if (mStateMgr.getOldStateID()->isEqual(StateID_Sleep)) {
        playChrAnim("dizzy_sleep", m3d::FORWARD_LOOP, 0.0f, 1.0f);
    } else {
        playChrAnim("dizzy", m3d::FORWARD_LOOP, 0.0f, 1.0f);
    }

    mSpeed.x = 0.0f;
    mSpeed.y = -4.0f;

    mTimer = 0;
}

void daEnHeiho_c::finalizeState_Dizzy() {}

void daEnHeiho_c::executeState_Dizzy() {
    updateModel();
    calcSpeedY();
    posMove();

    if (EnBgCheck() & 1) {
        mSpeed.y = 0.0f;
    }

    WaterCheck(mPos, 1.0f);

    mVec3_c effectPos(mPos.x, mPos.y + 24.0f, 0.0f);
    mVec3_c effectScale(1.0f, 1.0f, 1.0f);
    mDizzyEffect.createEffect("Wm_en_spindamage", 0, &effectPos, nullptr, &effectScale);

    if (mTimer > 600) {
        changeState(*const_cast<sStateIDIf_c*>(mStateMgr.getOldStateID()));
        mHealth = 1;
    }
    mTimer++;
}

void daEnHeiho_c::initializeState_Idle() {
    mSpeed.x = 0.0f;
}

void daEnHeiho_c::finalizeState_Idle() {}

void daEnHeiho_c::executeState_Idle() {
    updateModel();
    calcSpeedY();
    posMove();

    EnBgCheck();

    WaterCheck(mPos, 1.0f);
}