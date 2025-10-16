#include <kamek.h>
#include <new/bases/d_a_en_puchi_pakkun.hpp>
#include <new/new_profile.hpp>
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

const char* nipperArcList[] = {"pakkun_puchi", NULL};
const SpriteData nipperSpriteData = {fProfile::EN_PUCHI_PAKKUN, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c nipperProfile(&g_profile_EN_PUCHI_PAKKUN, "EN_PUCHI_PAKKUN", SpriteId::EN_PUCHI_PAKKUN, &nipperSpriteData, nipperArcList);

dCc_c::CcData_s l_nipper_cc = {
    0.0,                        // mOffsetX
    8.0,                        // mOffsetY
    8.0,                        // mWidth
    8.0,                        // mHeight
    dCc_c::CAT_ENTITY,          // mCategory
    0,                          // mAttackCategory
    0x4F,                       // mCategoryInteract
    0xFFBAD35E,                 // mAttackCategoryInteract     
    0,                          // mFlag
    &dEn_c::normal_collcheck,   // mCallback
};

int daEnPuchiPakkun_c::create() {
    // setup our model
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);
    mRes = dResMng_c::m_instance->mRes.getRes("pakkun_puchi", "g3d/pakkun_puchi.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("pakkun_puchi");
    mNipperModel.create(mdl, &mAllocator, 0x227, 1, nullptr);
    dActor_c::setSoftLight_Enemy(mNipperModel);

    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("attack");
    mAnmChr.create(mdl, resAnmChr, &mAllocator, 0);
    playChrAnim("attack", m3d::FORWARD_LOOP, 0.0, 1.0);

    mAllocator.adjustFrmHeap();

    // set y acceleration and max speed for gravity
    mAccelY = -0.1875;
    mSpeedMax.y = -4.0;

    // register the cc data (hitbox)
    mCc.registerCc(this, &l_nipper_cc);
    mCc.entry();

    mDirection = getPl_LRflag(mPos);
    mAngle.y = l_nipper_look_angle[mDirection];

    // assign settings variables
    mWalks = (mParam >> 17) & 1;
    mJumpHeight = (mParam >> 12) & 0xF;
    mSpitsFire = (mParam >> 1) & 1;

    // mCenterOffs is used to set the "center" of the actor
    // for yoshi tongue and dieFall
    mCenterOffs = mVec3_c(0.0, 8.0, 0.0);

    // set size for model culling
    mVisibleAreaSize.x = 16.0;
    mVisibleAreaSize.y = 16.0;
    mVisibleAreaOffset.x = 0.0;
    mVisibleAreaOffset.y = 8.0;

    // set yoshi eating behavior
    if (mSpitsFire) {
        mEatBehaviour = EAT_TYPE_FIREBALL;
    } else {
        mEatBehaviour = EAT_TYPE_EAT_PERMANENT;
    }

    // tile sensors
    static const dBcSensorLine_c below(-4<<12, 4<<12, 0<<12);
    static const dBcSensorPoint_c above(0<<12, 16<<12);
    static const dBcSensorLine_c adjacent(3<<12, 8<<12, 8<<12);

    mBc.set(this, (dBcSensor_c*)&below, (dBcSensor_c*)&above, (dBcSensor_c*)&adjacent);

    if (mParam & 1) { //spawn frozen
        changeState(StateID_IceWait);
        //create an AC_FREEZER at our position and layer
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
    // execute state and remove if outisde of zone
    mStateMgr.executeState();
    ActorScrOutCheck(0);
    return true;
}

int daEnPuchiPakkun_c::draw() {
    mNipperModel.entry();
    return SUCCEEDED;
}

void daEnPuchiPakkun_c::finalUpdate() {
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
    mNipperModel.setLocalMtx(&mMatrix);
    // see scale using boyon 
    mNipperModel.setScale(mBoyon.mScale.x, mBoyon.mScale.y, mBoyon.mScale.z);
    mNipperModel.calc(false);

    return;
}

void daEnPuchiPakkun_c::Normal_VsEnHitCheck(dCc_c *cc1, dCc_c *cc2) {
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

void daEnPuchiPakkun_c::initializeState_Ice() {
    if (!mWalks) {
        //munchers are slightly smaller when encased in ice, so we replicate that here
        mScale = mVec3_c(0.89, 0.89, 0.89);
        //store x pos
        mStoredIcePos = mPos.x;
        //set animation to open mouth
        mAnmChr.setFrame(0.0);
    }
    return dEn_c::initializeState_Ice();
}

void daEnPuchiPakkun_c::finalizeState_Ice() {
    //restore everything
    if (!mWalks) {
        mScale = mVec3_c(1.0, 1.0, 1.0);
        mPos.x = mStoredIcePos;
        playChrAnim("attack", m3d::FORWARD_LOOP, 0.0, 1.0);
    }
    return dEn_c::finalizeState_Ice();
}

void daEnPuchiPakkun_c::createIceActor() {
    // iceinfo is an array so that __destory_arr can be called
    dIceInfo nipperIceInfo[1] = {
        0x1000,                                 // mFlags
        mVec3_c(mPos.x, mPos.y, mPos.z+5.0),    // mPos
        mVec3_c(0.8, 0.8, 0.8),                 // mScale
        0.0, 
        0.0, 
        0.0, 
        0.0, 
        0.0, 
        0.0, 
        0.0
    };

    if (mWalks) {
        nipperIceInfo[0].mFlags = 0;
        nipperIceInfo[0].mPos = mVec3_c(mPos.x, mPos.y-1.0, mPos.z);
        nipperIceInfo[0].mScale = mVec3_c(1.0, 1.0, 1.0);
    }

    mIceMng.createIce(&nipperIceInfo[0], 1);
}

void daEnPuchiPakkun_c::playChrAnim(const char* name, m3d::playMode_e playMode, float blendFrame, float rate) {
	nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr(name);
	mAnmChr.setAnm(mNipperModel, resAnmChr, playMode);
	mNipperModel.setAnm(mAnmChr, blendFrame);
	mAnmChr.setRate(rate);

    //sync our animations with other nippers
    float frame = dScStage_c::m_exeFrame - (dScStage_c::m_exeFrame / int(mAnmChr.mFrameMax)) * int(mAnmChr.mFrameMax);
    mAnmChr.setFrame(frame);
}

void daEnPuchiPakkun_c::updateModel() {
    mNipperModel.play();
    return;
}

void daEnPuchiPakkun_c::setWalkSpeed() {
    mSpeed.x = l_nipper_walk_speed[mDirection];
    return;
}

bool daEnPuchiPakkun_c::checkForLedge(float xOffset) {
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

bool daEnPuchiPakkun_c::isPlayerAbove() {
    for (int i = 0; i < 4; i++) {
        dAcPy_c *player = daPyMng_c::getPlayer(i);
        //grab any active players
        if (player) {
            //are we in the same x range as the nipper?
            if (10.0 >= std::fabs(player->mPos.x - mPos.x)) {
                //are we in the y range?
                if (mPos.y + 8.0 <= player->mPos.y && player->mPos.y <= mPos.y + 104.0) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool daEnPuchiPakkun_c::isPlayerInFireRange() {
    for (int i = 0; i < 4; i++) {
        dAcPy_c *player = daPyMng_c::getPlayer(i);
        if (player) {
            if ((88.0 >= std::fabs(player->mPos.x - mPos.x)) && (48.0 >= abs(player->mPos.y - mPos.y))) {
                mFireDirection = getTrgToSrcDir_Main(player->mPos.x + player->mCenterOffs.x, mPos.x + mCenterOffs.x);
                setFireDistance(abs(player->mPos.x - mPos.x));
                return true;
            }
        }
    }
    return false;
}

void daEnPuchiPakkun_c::setFireDistance(float distance) {
    if (distance >= 65.0) {
        mFireDist = 3;
    } else if (distance >= 49.0) {
        mFireDist = 2;
    } else if (distance >= 30) {
        mFireDist = 1;
    } else {
        mFireDist = 0;
    }
}

void daEnPuchiPakkun_c::initializeState_Idle() {
    playChrAnim("attack", m3d::FORWARD_LOOP, 0.0, 1.0);
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

    killIfTouchingLava(mPos, 1.0);
}

void daEnPuchiPakkun_c::initializeState_Walk() {
    if (mStateMgr.getOldStateID() != &StateID_Turn) {
        playChrAnim("jump", m3d::FORWARD_LOOP, 0.0, 1.0);
    }
    setWalkSpeed();
}

void daEnPuchiPakkun_c::finalizeState_Walk() {}

void daEnPuchiPakkun_c::executeState_Walk() {
    updateModel();
    calcSpeedY();
    posMove();
    sLib::chaseAngle((short*)&mAngle.y, l_nipper_look_angle[mDirection], 0x500);

    if (!EnBgCheck() & 1) {
        if (mBc.isFoot() && (m49 == false) && (mSpeed.y <= 0.0f)) {
            mFootRelated2.x = mFootRelated2.x + m_1eb.x;
        }
    } else {
        if (checkForLedge(4.0f) == false) {
            changeState(StateID_Turn);
            return;
        }
        if (mBc.mFlags & 0x15 << mDirection & 0x3f) {
            mSpeed.x = 0.0;
            if (EnBgCheck() & 1) {
                changeState(StateID_Turn);
            }
        } else {
            mFootRelated2.x = 0.0;
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
    killIfTouchingLava(mPos, 1.0);
}

void daEnPuchiPakkun_c::initializeState_Turn() {
    if (mStateMgr.getOldStateID() != &StateID_Walk) {
        playChrAnim("jump", m3d::FORWARD_LOOP, 0.0, 1.0);
    } else {
        mDirection ^= 1;
    }
    mSpeed.x = 0.0;
}

void daEnPuchiPakkun_c::finalizeState_Turn() {
    mAngle.y = l_nipper_look_angle[mDirection];
}

void daEnPuchiPakkun_c::executeState_Turn() {
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

    if (dAudio::isBgmAccentSign(1)) {
        mIsBahJump = true;
        changeState(StateID_Jump);
    }

    killIfTouchingLava(mPos, 1.0);

    // face our new direction, and exit state when finished
    bool doneTurning = sLib::chaseAngle((short*)&mAngle.y, l_nipper_turn_angle[mDirection], 0x800);

    if (doneTurning) {
        changeState(StateID_Walk);
    }
    return;
}

void daEnPuchiPakkun_c::initializeState_Jump() {
    playChrAnim("jump", m3d::FORWARD_LOOP, 0.0, 1.0);

    mSpeed.x = 0.0;
    if (mIsBahJump) {
        mSpeed.y = 2.0;
    } else {
        switch (mJumpHeight) {
            default:
                mSpeed.y = 5.5; 
                break;
            case 1:
                mSpeed.y = 4.5;
                break;
            case 2:
                mSpeed.y = 3.5;
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

    killIfTouchingLava(mPos, 1.0);
}

void daEnPuchiPakkun_c::initializeState_IceWait() {
    mScale = mVec3_c(0.89, 0.89, 0.89);
    mAnmChr.setFrame(0.0);
}

void daEnPuchiPakkun_c::finalizeState_IceWait() {}

void daEnPuchiPakkun_c::executeState_IceWait() {
    u8 fireballState = 0;
    daFreezer_c *freezer;
    //make sure our AC_FREEZER still exists
    for (freezer = (daFreezer_c *)getConnectChild(); freezer != (daFreezer_c *)nullptr; freezer = (daFreezer_c *)freezer->getConnectBrNext()) {
        if (freezer != (daFreezer_c *)nullptr) {
            //if it does, check if it's melting
            fireballState = (freezer->mFireBallState != 0) + 1;
        }
    }
    //we're MELTING
    if (fireballState == 2) {
        mScale = mVec3_c(1.0, 1.0, 1.0);
    }
    //our AC_FREEZER no longer exists, so we've melted
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
    playChrAnim("spit", m3d::FORWARD_LOOP, 0.0, 1.0);
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
        mVec3_c firePos = mVec3_c(mPos.x, mPos.y + 10.0, mPos.z);
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