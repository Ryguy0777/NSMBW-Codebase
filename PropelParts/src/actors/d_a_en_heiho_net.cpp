#include <kamek.h>
#include <new/bases/d_a_en_heiho_net.hpp>
#include <new/bases/d_custom_profile.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <constants/sound_list.h>

CUSTOM_ACTOR_PROFILE(EN_HEIHO_NET, daEnHeihoNet_c, fProfile::EN_NET_NOKONOKO_UD, fProfile::DRAW_ORDER::EN_NET_NOKONOKO_UD, 0x12);

STATE_DEFINE(daEnHeihoNet_c, MoveUD);
STATE_DEFINE(daEnHeihoNet_c, Turn_St);
STATE_DEFINE(daEnHeihoNet_c, Turn_Ed);
STATE_DEFINE(daEnHeihoNet_c, MoveLR);
STATE_DEFINE(daEnHeihoNet_c, Turn);
STATE_DEFINE(daEnHeihoNet_c, Attack);

const char* l_HEIHO_NET_res[] = {"heiho_net", NULL};
const dActorData_c c_HEIHO_NET_actor_data = {fProfile::EN_HEIHO_NET, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c l_HEIHO_NET_profile(&g_profile_EN_HEIHO_NET, "EN_HEIHO_NET", CourseActor::EN_HEIHO_NET, &c_HEIHO_NET_actor_data, l_HEIHO_NET_res);

const float daEnHeihoNet_c::smc_CLIMB_SPEED = 0.5f;

const s16 l_heiho_net_angleY[] = { 0x8000, 0x0000 };
const float l_heiho_net_Z_pos[] = { 1500.0, -2500.0 };
const mVec2_c l_heiho_net_offset_UD[] = { mVec2_c(0.0, -2.0), mVec2_c(0.0, -14.0) };
const mVec2_c l_heiho_net_offset_LR[] = { mVec2_c(8.0, 0.0), mVec2_c(-8.0, 0.0) };
const m3d::playMode_e l_heiho_net_playmode[] = { m3d::FORWARD_LOOP, m3d::REVERSE_LOOP };

const sBcSensorPoint l_heiho_net_head = { 0, 0x0, 0x14000 };
const sBcSensorLine l_heiho_net_foot = { 1, -0x4000, 0x4000, 0 };
const sBcSensorLine l_heiho_net_wall = { 1, 0x3000, 0x8000, 0x8000 };

const sCcDatNewF l_heiho_net_cc = {
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

int daEnHeihoNet_c::create() {
    // Assign settings variables
    mAmiLayer = mParam >> 16 & 1;

    mColor = mParam >> 24 & 0xF;
    mAttacks = mParam >> 19 & 1;
    mVertical = mParam >> 18 & 1;
    
    // Setup our model
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);
    
    loadModel();

    mAllocator.adjustFrmHeap();

    // Register the cc data (hitbox)
    mCc.set(this, (sCcDatNewF *)&l_heiho_net_cc);
    mCc.entry();

    // Set spawning direction
    if (mVertical) {
        mDirection = getPl_UDflag(mPos);
    } else {
        mDirection = getPl_LRflag(mPos);
    }
    mTurnDirection = mDirection;

    mAngle.y = l_heiho_net_angleY[mAmiLayer];

    // Tile sensors
    mBc.set(this, l_heiho_net_head, l_heiho_net_head, l_heiho_net_wall);

    // mCenterOffs is used to set the "center" of the actor
    // For yoshi tongue and dieFall
    mCenterOffs.set(0.0f, 12.0f, 0.0f);

    // Set size for model culling
    mVisibleAreaSize.set(16.0f, 24.0f);
    mVisibleAreaOffset.set(0.0f, 12.0f);

    // Set yoshi eating behavior
    mEatBehaviour = EAT_TYPE_EAT_PERMANENT;

    mIceMng.setIceStatus(0, 3, 3);

    setMoveState();

    return SUCCEEDED;
}

int daEnHeihoNet_c::draw() {
    mHeihoModel.entry();

    return SUCCEEDED;
}

void daEnHeihoNet_c::finalUpdate() {
    calcMdl();
}

void daEnHeihoNet_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *pl = (dActor_c*)other->mpOwner;
    // Enfumi_check returns the collision type of the player and enemy 
    int fumi_check = Enfumi_check(self, other, 0);
    if (fumi_check == 1) { // Regular jump
        mVec2_c killSpeed(0.0, 0.0);
        return setDeathInfo_Fumi(pl, killSpeed, dEn_c::StateID_DieFumi, 0);
    } else if (fumi_check == 3) { // Spin jump
        return setDeathInfo_SpinFumi(pl, 1);
    } else if (fumi_check == 0) { // Hit from the sides
        return dEn_c::Normal_VsPlHitCheck(self, other);
    }
}

void daEnHeihoNet_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *pl = (dActor_c*)other->mpOwner;
    int fumi_check = Enfumi_check(self, other, 0);
    if (fumi_check == 1) {
        return setDeathInfo_YoshiFumi(pl);
    } else if (fumi_check == 0) {
        return dEn_c::Normal_VsYoshiHitCheck(self, other);
    }
}

void daEnHeihoNet_c::initializeState_DieFumi() {
    playChrAnim("diefall", m3d::FORWARD_LOOP, 0.0f, 1.0f);
    return dEn_c::initializeState_DieFumi();
}

void daEnHeihoNet_c::executeState_DieFumi() {
    mdlPlay();
    return dEn_c::executeState_DieFumi();
}

void daEnHeihoNet_c::initializeState_DieFall() {
    playChrAnim("diefall", m3d::FORWARD_LOOP, 0.0f, 1.0f);
    return dEn_c::initializeState_DieFall();
}

void daEnHeihoNet_c::executeState_DieFall() {
    mdlPlay();
    return dEn_c::executeState_DieFall();
}

float l_heiho_net_Z_offset[2] = {1.5, -1.5};

bool daEnHeihoNet_c::createIceActor() {
    // Iceinfo is an array so that __destory_arr can be called
    dIceInfo heihoNetIceInfo[1] = {
        0,                                              // mFlags
        mVec3_c(mPos.x, mPos.y-3.8f,                    // mPos
            mPos.z+l_heiho_net_Z_offset[mAmiLayer]),
        mVec3_c(1.3f, 1.5f, 1.6f),                      // mScale
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f, 
        0.0f
    };
    return mIceMng.createIce(&heihoNetIceInfo[0], 1);
}

void daEnHeihoNet_c::finalizeState_NetMove() {
    if (mVertical) {
        mAmiLayer ^= 1;
        mPos.z = l_heiho_net_Z_pos[mAmiLayer];
        mCc.mAmiLine = l_Ami_Line[mAmiLayer];
        mBc.mAmiLine = l_Ami_Line[mAmiLayer];
        mCc.mCcData.mCallback = &dEn_c::normal_collcheck;
    } else {
        return daNetEnemy_c::finalizeState_NetMove();
    }
}

void daEnHeihoNet_c::mdlPlay() {
    mHeihoModel.play();
}

void daEnHeihoNet_c::calcMdl() {
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

const char *l_heiho_net_anims[2] = {"climb_lr", "climb_ud"};

void daEnHeihoNet_c::loadModel() {
    mRes = dResMng_c::m_instance->getRes("heiho_net", "g3d/heiho.brres");
    mResMdl = mRes.GetResMdl("heiho");
    mHeihoModel.create(mResMdl, &mAllocator, 0x23, 1, nullptr);
    setSoftLight_Enemy(mHeihoModel);

    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr(l_heiho_net_anims[mVertical]);
    mAnmChr.create(mResMdl, resAnmChr, &mAllocator, 0);
    playClimbAnim();

    nw4r::g3d::ResAnmTexPat resPat = mRes.GetResAnmTexPat("color");
    mAnmTexPat.create(mResMdl, resPat, &mAllocator, 0, 1);
    mAnmTexPat.setAnm(mHeihoModel, resPat, 0, m3d::FORWARD_ONCE);
    mAnmTexPat.setRate(0.0f, 0);
    mAnmTexPat.setFrame(mColor, 0);
    mHeihoModel.setAnm(mAnmTexPat);
}

void daEnHeihoNet_c::playChrAnim(const char* name, m3d::playMode_e playMode, float blendFrame, float rate) {
    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr(name);
    mAnmChr.setAnm(mHeihoModel, resAnmChr, playMode);
    mHeihoModel.setAnm(mAnmChr, blendFrame);
    mAnmChr.setRate(rate);
}

void daEnHeihoNet_c::playClimbAnim() {
    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr(l_heiho_net_anims[mVertical]);
    m3d::playMode_e playMode = m3d::FORWARD_LOOP;
    if (mDirection == 0 && !mVertical || mDirection == 1 && mVertical) {
        playMode = m3d::REVERSE_LOOP;
    }
    mAnmChr.setAnm(mHeihoModel, resAnmChr, playMode);
    mHeihoModel.setAnm(mAnmChr, 0.0f);
    mAnmChr.setRate(0.91f);
}

int daEnHeihoNet_c::wireBgCheck(const mVec2_c &offset) {
    mVec3_c pos = getCenterPos();
    pos.x += offset.x;
    pos.y += offset.y;

    int unitType = dBc_c::getUnitType(pos.x, pos.y, mLayer);
    if (unitType & 1024) {
        int unitKind = dBc_c::getUnitKind(pos.x, pos.y, mLayer);
        if ((unitKind & 0xFF) > 1) {
            return unitKind & 0xFF;
        }
    }
    return -1;
}

void daEnHeihoNet_c::setMoveState() {
    if (mVertical) {
        changeState(StateID_MoveUD);
    } else {
        changeState(StateID_MoveLR);
    }
}

void daEnHeihoNet_c::checkAttack() {
    if (mAttacks) {
        if (mAttackTimer > 0) {
            mAttackTimer--;
        } else {
            if (isPlayerInAttackRange()) {
                changeState(StateID_Attack);
            }
        }
    }
}

bool daEnHeihoNet_c::isPlayerInAttackRange() {
    for (int i = 0; i < PLAYER_COUNT; i++) {
        dAcPy_c *player = daPyMng_c::getPlayer(i);
        if (player) {
            if (player->mAmiLayer != mAmiLayer) {
                if ((8.0f >= abs(player->mPos.x - mPos.x)) && (8.0f >= abs(player->mPos.y - mPos.y))) {
                    return true;
                }
            }
        }
    }
    return false;
}

void daEnHeihoNet_c::attackPlayer() {
    for (int i = 0; i < PLAYER_COUNT; i++) {
        dAcPy_c *player = daPyMng_c::getPlayer(i);
        if (player) {
            if (player->mAmiLayer != mAmiLayer) {
                if ((16.0f >= abs(player->mPos.x - mPos.x)) && (16.0f >= abs(player->mPos.y - mPos.y))) {
                    setDamage(player);
                }
            }
        }
    }
}

void daEnHeihoNet_c::initializeState_MoveUD() {
    const sStateIDIf_c *prevState = mStateMgr.getOldStateID();
    if (prevState != &StateID_NetWait && prevState != &StateID_NetMove) {
        if (prevState != &StateID_Turn) {
            playClimbAnim();
        }
        static const float dirSpeed[] = { smc_CLIMB_SPEED, -smc_CLIMB_SPEED };
        mSpeed.y = dirSpeed[mDirection];
        mPos.z = l_heiho_net_Z_pos[mAmiLayer];
        mCc.mAmiLine = l_Ami_Line[mAmiLayer];
        mBc.mAmiLine = l_Ami_Line[mAmiLayer];
        mTargetYPos = 0.0f;
    }
}

void daEnHeihoNet_c::finalizeState_MoveUD() {}

void daEnHeihoNet_c::executeState_MoveUD() {
    mdlPlay();
    posMove();
    checkAttack();
    if (mDirection == 0) {
        if (!mBc.checkHead(0)) {
            if (wireBgCheck(l_heiho_net_offset_UD[0]) == -1) {
                changeState(StateID_Turn_St);
            }
        } else {
            mDirection = 1;
            mTurnDirection = 1;
            mPos.y = mLastPos.y;
            mSpeed.y = -mSpeed.y;
            mAnmChr.mPlayMode = l_heiho_net_playmode[1];
        }
    } else {
        if (wireBgCheck(l_heiho_net_offset_UD[1]) == -1) {
            mDirection = 0;
            mTurnDirection = 0;
            mPos.y = mLastPos.y;
            mSpeed.y = -mSpeed.y;
            mAnmChr.mPlayMode = l_heiho_net_playmode[0];
        }
    }
}

void daEnHeihoNet_c::initializeState_Turn_St() {
    const sStateIDIf_c *prevState = mStateMgr.getOldStateID();
    if (prevState != &StateID_NetWait && prevState != &StateID_NetMove) {
        if (prevState == &StateID_MoveUD) {
            playChrAnim("turn_st", m3d::FORWARD_ONCE, 0.0f, 0.91f);
            mSpeed.x = 0.0;
            mDirection ^= 1;
            mAmiLayer ^= 1;
            mTargetYPos = mPos.y;
            mCc.mAmiLine = l_Ami_Line[mAmiLayer];
            mBc.mAmiLine = l_Ami_Line[mAmiLayer];
        }
    }
}

void daEnHeihoNet_c::finalizeState_Turn_St() {}

void daEnHeihoNet_c::executeState_Turn_St() {
    mdlPlay();
    if (mAnmChr.isStop()) {
        changeState(StateID_Turn_Ed);
    }
}

void daEnHeihoNet_c::initializeState_Turn_Ed() {
    const sStateIDIf_c *prevState = mStateMgr.getOldStateID();
    if (prevState != &StateID_NetWait && prevState != &StateID_NetMove) {
        if (prevState == &StateID_Turn_St) {
            playChrAnim("turn_ed", m3d::FORWARD_ONCE, 0.0f, 0.0f);
            mSpeed.y = 2.2f;
        }
    }
}

void daEnHeihoNet_c::finalizeState_Turn_Ed() {}

void daEnHeihoNet_c::executeState_Turn_Ed() {
    mdlPlay();
    s16 targetAng = (l_base_angleY_add[mDirection] * l_EnMuki[mAmiLayer]);
    if (mAngle.y != l_heiho_net_angleY[mAmiLayer]) {
        targetAng += mAngle.y;
        mAngle.y = targetAng;
        if (targetAng == 0x4000 || targetAng == -0x4000) {
            mPos.z = l_heiho_net_Z_pos[mAmiLayer];
        }
    }
    calcSpeedY();
    posMove();
    if (mPos.y < mTargetYPos) {
        mAnmChr.setRate(0.91);
        mPos.y = mTargetYPos;
        mAngle.y = l_heiho_net_angleY[mAmiLayer];
        if (mAnmChr.isStop()) {
            changeState(StateID_MoveUD);
        }
    }
}

void daEnHeihoNet_c::initializeState_MoveLR() {
    const sStateIDIf_c *prevState = mStateMgr.getOldStateID();
    if (prevState != &StateID_NetWait && prevState != &StateID_NetMove) {
        if (prevState != &StateID_Turn) {
            playClimbAnim();
        }
        static const float dirSpeed[] = { smc_CLIMB_SPEED, -smc_CLIMB_SPEED };
        mSpeed.x = dirSpeed[mDirection];
        mPos.z = l_heiho_net_Z_pos[mAmiLayer];
        mCc.mAmiLine = l_Ami_Line[mAmiLayer];
        mBc.mAmiLine = l_Ami_Line[mAmiLayer];
        mTargetYPos = 0.0f;
    }
}

void daEnHeihoNet_c::finalizeState_MoveLR() {}

void daEnHeihoNet_c::executeState_MoveLR() {
    mdlPlay();
    posMove();
    mBc.checkWall(nullptr);
    if (mBc.mFlags & 0x15 << mDirection & 0x3f) {
        mDirection ^= 1;
        mTurnDirection ^= 1;
        mTargetYPos = 0.0f;
        mSpeed.x = -mSpeed.x;
    } else {
        checkAttack();
        if (wireBgCheck(l_heiho_net_offset_LR[mDirection]) == -1) {
            mPos.x = mLastPos.x;
            changeState(StateID_Turn);
        }
    }
}

void daEnHeihoNet_c::initializeState_Turn() {
    const sStateIDIf_c *prevState = mStateMgr.getOldStateID();
    if (prevState != &StateID_NetWait && prevState != &StateID_NetMove) {
        // Originally this was done through math, since the koopas have variable speed
        s8 turnMulti = l_EnMuki[mTurnDirection];
        mTurnTimer = 50;
        mTurnStep = turnMulti * (0x8000 / 50);
    }
}

void daEnHeihoNet_c::finalizeState_Turn() {}

void daEnHeihoNet_c::executeState_Turn() {
    mdlPlay();
    posMove();
    mTurnTimer--;
    mAngle.y += mTurnStep;
    if (mTurnTimer == 0) {
        mAngle.y = l_heiho_net_angleY[mAmiLayer];
        changeState(StateID_MoveLR);
    } else if (mTurnTimer == 25) {
        mDirection ^= 1;
        mAngle.y = ((l_EnMuki[mDirection]) << 0xE);
        mSpeed.x = -mSpeed.x;
        mAmiLayer ^= 1;
        mCc.mAmiLine = l_Ami_Line[mAmiLayer];
        mBc.mAmiLine = l_Ami_Line[mAmiLayer];
        mPos.z = l_heiho_net_Z_pos[mAmiLayer];
    }
}

void daEnHeihoNet_c::initializeState_Attack() {
    mAttackTimer = 150;
    playChrAnim("attack", m3d::FORWARD_ONCE, 0.0f, 1.0f);
    mSpeed.set(0.0f, 0.0f, 0.0f);
}

void daEnHeihoNet_c::finalizeState_Attack() {}

void daEnHeihoNet_c::executeState_Attack() {
    mdlPlay();
    if (mAnmChr.getFrame() == 10.0f) {
        nw4r::g3d::ResNode root = mResMdl.GetResNode("skl_root");
        int rootNodeID = root.GetID();
        mVec3_c handPos;
        mHeihoModel.getNodeWorldMtxMultVecZero(rootNodeID, handPos);

        mEf::createEffect("Wm_mr_wirehit", 0, &handPos, nullptr, nullptr);
        mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
        dAudio::g_pSndObjEmy->startSound(SE_PLY_ATTACK_FENCE, soundPos, 0);
    }

    if (mAnmChr.getFrame() > 10.0) {
        attackPlayer();
    }

    if (mAnmChr.isStop()) {
        setMoveState();
    }
} 

// Patch the fence flippers to support our new actors

// The game does a hardcoded check on 3 different actors when a flip panel is flipped
// Amps (unused, might be a DS leftover) and the Climbing Koopas
// We add a check for the fence ShyGuys
extern "C" void checkForKoopas(void);
kmBranchDefAsm(0x80aca5b8, 0x80aca5cc) {
    nofralloc

    cmplwi r3, 774 // EN_HEIHO_NET
    beq isFenceActor
    addis r3, r3, 1 // Original instruction
    b checkForKoopas

    isFenceActor:
    blr
}

// This check does... something, not really sure what
extern "C" void checkForKoopas2(void);
kmBranchDefAsm(0x80868710, 0x80868724) {
    nofralloc

    cmplwi r3, 774 // EN_HEIHO_NET
    beq isFenceActor2
    addis r3, r3, 1 // Original instruction
    b checkForKoopas2

    isFenceActor2:
    blr
}