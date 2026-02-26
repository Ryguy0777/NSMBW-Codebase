#include <kamek.h>
#include <new/bases/d_a_en_kuribon.hpp>
#include <new/bases/d_custom_profile.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_audio.hpp>
#include <constants/sound_list.h>
#include <game/bases/d_a_player_manager.hpp>

CUSTOM_ACTOR_PROFILE(EN_KURIBON, daEnKuribon_c, fProfile::EN_KURIBO, fProfile::DRAW_ORDER::EN_KURIBO, 0x12);

STATE_DEFINE(daEnKuribon_c, Dizzy);
STATE_DEFINE(daEnKuribon_c, Carry);
STATE_DEFINE(daEnKuribon_c, Throw);
STATE_DEFINE(daEnKuribon_c, Kick);
STATE_DEFINE(daEnKuribon_c, Recover);

const char* l_KURIBON_res[] = {"kuribon", NULL};
const dActorData_c c_KURIBON_actor_data = {fProfile::EN_KURIBON, 8, -16, 0, 8, 8, 8, 0, 0, 0, 0, 0};
dCustomProfile_c l_KURIBON_profile(&g_profile_EN_KURIBON, "EN_KURIBON", CourseActor::EN_KURIBON, &c_KURIBON_actor_data, l_KURIBON_res);

const float daEnKuribon_c::smc_WALK_SPEED = 0.5f;
const float daEnKuribon_c::smc_THROW_SPEED_X = 4.0f;
const float daEnKuribon_c::smc_KICK_SPEED_X = 2.5f;
const float daEnKuribon_c::smc_KICK_SPEED_Y = 2.0f;

const s16 l_kuribon_carry_angleY[] = { -0x6000, 0x6000 };

bool daEnKuribon_c::isSpinLiftUpEnable() {
    return isState(StateID_Dizzy);
}

void daEnKuribon_c::setSpinLiftUpActor(dActor_c *carryingActor) {
    mCarryID = *carryingActor->getPlrNo();
    mCarryPos.set(0.0f, 0.0f, 0.0f);
    changeState(StateID_Carry);
}

void daEnKuribon_c::Normal_VsEnHitCheck(dCc_c *self, dCc_c *other) {
    u16 flag = other->mCcData.mStatus;
    dActor_c *ac = other->mpOwner;
    
    if (self->mCcData.mAttack == CC_ATTACK_SHELL && (flag & 0x100) && hitCallback_Shell(self, other)) {
        // If we're throwing or kicking, kill the colliding actor
        other->mInfo |= CC_NO_HIT;
        self->mInfo |= CC_NO_HIT;
    } else if (
        isState(StateID_Carry) && (other->mCcData.mVsDamage & BIT_FLAG(CC_ATTACK_SHELL)) &&
        ac->mProfName != fProfile::EN_HATENA_BALLOON && hitCallback_Shell(self, other)
    ) {
        // If we're carrying, kill both actors
        other->mInfo |= CC_NO_HIT;
    } else if (isState(StateID_Walk)) {
        // If we're walking, turn
        float offsetX = self->mCollOffsetX[3];

        if ((mDirection == 1 && offsetX > 0.0f) || (mDirection == 0 && offsetX < 0.0f)) {
            setTurnByEnemyHit();
        }
    } else {
        dEn_c::Normal_VsEnHitCheck(self, other);
    }
}

void daEnKuribon_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    dAcPy_c *pl = (dAcPy_c *) other->mpOwner;
    if (fManager_c::searchBaseByID(pl->mCarryActorID) == this) {
        return;
    }
    u8 newDir = self->mCollOffsetX[0] >= 0.0f ? 0 : 1;
    u8 plrNo = *pl->getPlrNo();
    if (isState(StateID_Dizzy)) {
        if (carry_check(pl)) {
            mCarryID = plrNo;
            mCarryPos.set(0.0f, -5.0f, 6.0f);
            changeState(StateID_Carry);
        } else {
            mDirection = newDir;
            mNoHitPlayer.mTimer[plrNo] = 32;
            playKickSound(plrNo);
            mPlayerNo = plrNo;
            mBc.mOwningPlayerNo = plrNo;
            changeState(StateID_Kick);
        }
    } else if (isState(StateID_Throw) || isState(StateID_Kick)) {
        mDirection = newDir;
        mNoHitPlayer.mTimer[plrNo] = 32;
        playKickSound(plrNo);
        mPlayerNo = plrNo;
        mBc.mOwningPlayerNo = plrNo;
        changeState(StateID_Kick);
    } else {
        int checkRes = Enfumi_check(self, other, 0);
        if (checkRes == 0) {
            if (!isState(StateID_Carry)) {
                dEn_c::Normal_VsPlHitCheck(self, other);
            }
        } else if (checkRes == 1) {
            mPlayerNo = plrNo;
            mBc.mOwningPlayerNo = plrNo;
            if (isState(StateID_Carry)) {
                changeState(StateID_Kick);
            } else {
                reactFumiProc(pl);
            }
        } else if (checkRes == 3) {
            reactSpinFumiProc(pl);
        }
    }
}

void daEnKuribon_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    daYoshi_c *ys = (daYoshi_c *) other->mpOwner;
    u8 newDir = self->mCollOffsetX[2] >= 0.0f ? 0 : 1;
    u8 plrNo = *ys->getPlrNo();
    if (Enfumi_check(self, other, 0) == 0) {
        if (isState(StateID_Dizzy) || isState(StateID_Throw) || isState(StateID_Kick)) {
            if (plrNo < PLAYER_COUNT) {
                mDirection = newDir;
                mPlayerNo = plrNo;
                mNoHitPlayer.mTimer[plrNo] = 32;
                mBc.mOwningPlayerNo = plrNo;
                playKickSound(plrNo);
                changeState(StateID_Kick);
            }
        } else {
            daEnKuriboBase_c::Normal_VsYoshiHitCheck(self, other);
        }
    } else {
        reactYoshiFumiProc(ys);
    }
}

void daEnKuribon_c::initializeState_DieOther() {
    return dEn_c::initializeState_DieOther();
}

void daEnKuribon_c::executeState_DieOther() {
    return dEn_c::executeState_DieOther();
}

void daEnKuribon_c::finalizeState_DieOther() {
    return dEn_c::finalizeState_DieOther();
}

void daEnKuribon_c::initializeState_Walk() {
    if (mStateMgr.getOldStateID() != &StateID_Turn) {
        setWalkAnm();
    }
    setWalkSpeed();
    mAccelY = -0.1875f;
    mSpeedMax.y = -4.0f;
}

void daEnKuribon_c::calcModel() {
    if (isState(StateID_Carry)) {
        mPos = calcCarryPos(mCarryPos);
    }
    return daEnKuribo_c::calcModel();
}

void daEnKuribon_c::reactFumiProc(dActor_c *player) {
    changeState(StateID_Dizzy);
}

void daEnKuribon_c::createBodyModel() {
    mRes = dResMng_c::m_instance->getRes("kuribon", "g3d/kuribon.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("kuribon");
    mModel.create(mdl, &mAllocator, 0x20, 1, nullptr);
    setSoftLight_Enemy(mModel);

    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk");
    mAnmChr.create(mdl, resAnmChr, &mAllocator, 0);
}

void daEnKuribon_c::initialize() {
    mCc.mCcData.mVsDamage = 0xFFBEFFFE;
    return daEnKuribo_c::initialize();
}

void daEnKuribon_c::setWalkAnm() {
    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("walk");
    mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmChr, 2.0f);
    mAnmChr.setRate(2.0f);
}

void daEnKuribon_c::playWalkAnm() {
    mModel.play();
}

void daEnKuribon_c::updateCarryCc() {
    bool dir = mAngle.y < 0 ? 1 : 0;

    static const float fs[] = { -8.0f, 8.0f };

    mVec3_c v1(
        getCenterPos().x + fs[dir],
        getCenterPos().y,
        getCenterPos().z
    );

    mVec3_c v2(
        v1.x + l_EnMuki[dir] * 16.0f,
        v1.y,
        v1.z
    );

    mCc.mCcData.mOffset.set(mCcOffsetX, mCcOffsetY);
    mCc.mCcData.mSize.set(mCcWidth, mCcHeight);
    float f = 0.0f;
    if (dBc_c::checkWall(&v1, &v2, &f, mLayer, 1, nullptr)) {
        float f1 = (v1.x + f) * 0.5f;
        float f2 = (v1.x - f) * 0.5f;
        mCc.mCcData.mOffset.set(f1 - mPos.x, 8.0f);
        mCc.mCcData.mSize.set(std::fabs(f2), 8.0f);
    }
}

void daEnKuribon_c::setDeathInfo_Carry(dActor_c *killedBy) {
    sDeathInfoData deathInfo = {
        0.0f,
        3.0f,
        -4.0f,
        -0.1875f,
        &StateID_DieFall,
        -1,
        -1,
        mDirection,
        (u8) *killedBy->getPlrNo()
    };
    mDeathInfo = deathInfo;
}

void daEnKuribon_c::checkTimer() {
    if (mFlipTimer >= 600) {
        changeState(StateID_Recover);
    } else if (mFlipTimer >= 480) {
        mAnmChr.setRate(2.0f);
    } else {
        mAnmChr.setRate(1.0f);
    }
    mFlipTimer++;
}

void daEnKuribon_c::playKickSound(int player_id) {
    mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjEmy->startSound(SE_EMY_KAME_KERU, soundPos, dAudio::getRemotePlayer(player_id));
}

void daEnKuribon_c::initializeState_Dizzy() {
    const sStateIDIf_c *prevState = mStateMgr.getOldStateID();
    if (prevState != &StateID_Dizzy && prevState != &StateID_Kick && prevState != &StateID_Throw) {
        nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("ReverseSwoon");
        mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
        mModel.setAnm(mAnmChr, 0.0f);
        mAnmChr.setRate(1.0f);
    }

    mAngle.x = 0x8000;
    mAngle.y = l_kuribon_carry_angleY[mDirection];
    mSpeed.x = 0.0f;
    mAccelY = -0.1875f;
    mSpeedMax.y = -4.0f;
}

void daEnKuribon_c::finalizeState_Dizzy() {};

void daEnKuribon_c::executeState_Dizzy() {
    checkTimer();
    playWalkAnm();
    calcSpeedY();
    posMove();

    if (EnBgCheck() & 1) {
        if (isOnEnLiftRemoconTrpln()) {
            // Bounce if on Bouncy Cloud platform
            mSpeed.y = 5.5f;
        } else {
            mSpeed.y = 0.0f;
        }
    }

    WaterCheck(mPos, 1.0f);
}

void daEnKuribon_c::initializeState_Carry() {
    mPlayerNo = mCarryID;
    dAcPy_c *player = daPyMng_c::getPlayer(mCarryID);
    mAmiLayer = player->mAmiLayer;

    mCcOffsetX = mCc.mCcData.mOffset.x;
    mCcOffsetY = mCc.mCcData.mOffset.y;
    mCcWidth = mCc.mCcData.mSize.x;
    mCcHeight = mCc.mCcData.mSize.y;
    mCc.mAmiLine = l_Ami_Line[mAmiLayer];
    mBc.mAmiLine = l_Ami_Line[mAmiLayer];
    mCc.mCcData.mVsKind |= BIT_FLAG(CC_KIND_KILLER) | BIT_FLAG(CC_KIND_BALLOON);
    mCc.mCcData.mAttack = CC_ATTACK_SHELL;
    mRc.setRide(nullptr);
    mActorProperties &= ~0x2;
}


void daEnKuribon_c::finalizeState_Carry() {
    dAcPy_c *player = daPyMng_c::getPlayer(mCarryID);
    player->cancelCarry(this);
    mCc.mCcData.mVsKind &= ~BIT_FLAG(CC_KIND_KILLER);
    mCc.mCcData.mAttack = CC_ATTACK_NONE;
    mCc.mCcData.mOffset.set(mCcOffsetX, mCcOffsetY);
    mCc.mCcData.mSize.set(mCcWidth, mCcHeight);
    mRc.setRide(nullptr);
    mBc.mFlags = 0;
    mCarryingFlags &= ~(CARRY_RELEASE | CARRY_THROW);
    mActorProperties |= 0x2;
    mAngle.y = l_kuribon_carry_angleY[mDirection];
}

void daEnKuribon_c::executeState_Carry() {
    checkTimer();
    dAcPy_c *player = daPyMng_c::getPlayer(mCarryID);
    playWalkAnm();
    
    if (mCarryingFlags & CARRY_RELEASE) {
        mDirection = mThrowDirection;
        mAngle.y = l_kuribon_carry_angleY[mDirection];
        if (checkWallAndBg()) {
            setDeathInfo_Carry(player);
            return;
        }
        if ((mBc.mFlags & 0x15 << mDirection) == 0) {
            mPos.x += l_EnMuki[mDirection] * 6.0f;
        }
        if (mCarryingFlags & CARRY_THROW) {
            changeState(StateID_Throw);
        } else {
            changeState(StateID_Dizzy);
        }
    } else {
        mAngle.y = (s16)player->mAngle.y + 0x8000;
        checkWallAndBg();
        WaterCheck(mPos, 1.0f);
    }
    updateCarryCc();
}

void daEnKuribon_c::initializeState_Throw() {
    static const float dirSpeed[] = { smc_THROW_SPEED_X, -smc_THROW_SPEED_X };
    clrComboCnt();
    mAccelF = 0.0625f;
    mSpeed.x = dirSpeed[mDirection];
    mSpeedMax.x = 0.0f;
    mAngle.y = l_kuribon_carry_angleY[mDirection];
    mNoHitPlayer.mTimer[mPlayerNo] = 10;
    mCc.mCcData.mAttack = CC_ATTACK_SHELL;
}

void daEnKuribon_c::finalizeState_Throw() {
    clrComboCnt();
    mAccelF = 0.0f;
    mSpeedMax.x = 0.0f;
    mCc.mCcData.mAttack = CC_ATTACK_NONE;
}

void daEnKuribon_c::executeState_Throw() {
    checkTimer();
    playWalkAnm();
    calcSpeedX();
    calcSpeedY();
    posMove();
    u32 bcFlags = EnBgCheckFoot();
    if (mBc.isFoot()) {
        Bound(0.1875f, 0.65f, 0.5f);
        if (std::fabs(mSpeed.x) > 0.2f) {
            mVec3_c efPos(mPos.x, mPos.y, 5500.0f);
            mEf::createEffect("Wm_en_landsmoke_s", 0, &efPos, nullptr, nullptr);
        }
    }
    mBc.checkHead(bcFlags);
    EnBgCheckWall();
    if (mBc.mFlags & 0x15 << mDirection) {
        mDirection ^= 1;
        mAngle.y = l_kuribon_carry_angleY[mDirection];
        mSpeed.x = -mSpeed.x * smc_WALK_SPEED;
    }
    WaterCheck(mPos, 1.0f);
    if (mSpeed.x == 0.0f) {
        changeState(StateID_Dizzy);
    }
}

void daEnKuribon_c::initializeState_Kick() {
    mAnmChr.setRate(2.0f);
    static const float dirSpeed[] = { smc_KICK_SPEED_X, -smc_KICK_SPEED_X };

    mSpeed.set(dirSpeed[mDirection], smc_KICK_SPEED_Y, 0.0f);
    mAngle.y = l_kuribon_carry_angleY[mDirection];
    clrComboCnt();
    mAccelY = -0.1875f;
    mCc.mCcData.mAttack = CC_ATTACK_SHELL;
}

void daEnKuribon_c::finalizeState_Kick() {
    clrComboCnt();
    mCc.mCcData.mAttack = CC_ATTACK_NONE;
}

void daEnKuribon_c::executeState_Kick() {
    checkTimer();
    playWalkAnm();
    calcSpeedX();
    calcSpeedY();
    posMove();
    u32 bcFlags = EnBgCheckFoot();
    if (mBc.isFoot()) {
        Bound(0.1875f, 0.5f, 0.5f);
    }
    mBc.checkHead(bcFlags);
    EnBgCheckWall();
    if (bcFlags != 0) {
        if (std::fabs(mSpeed.x) > 0.2f) {
            mVec3_c efPos(mPos.x, mPos.y, 5500.0f);
            mEf::createEffect("Wm_en_landsmoke_s", 0, &efPos, nullptr, nullptr);
        }
    }
    if (mBc.mFlags & 0x3c000000) {
        if (mSpeed.y > 0.0f) {
            mSpeed.y = -0.85f * mSpeed.y;
        }
    }
    if (mBc.mFlags & 0x15 << mDirection) {
        mDirection = mDirection ^ 1;
        mAngle.y = l_kuribon_carry_angleY[mDirection];
        mSpeed.x = -mSpeed.x;
    }
    WaterCheck(mPos, 1.0f);
    if (mSpeed.x == 0.0f) {
        changeState(StateID_Dizzy);
    }
}

void daEnKuribon_c::initializeState_Recover() {
    nw4r::g3d::ResAnmChr resAnmChr = mRes.GetResAnmChr("ReverseRecover");
    mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_ONCE);
    mModel.setAnm(mAnmChr, 0.0f);
    mAnmChr.setRate(2.0f);

    mFlipTimer = 0;
    mSpeed.set(0.0f, 0.0f, 0.0f);
    mAccelY = -0.1875f;
    mSpeedMax.y = -4.0f;
}

void daEnKuribon_c::finalizeState_Recover() {}

void daEnKuribon_c::executeState_Recover() {
    playWalkAnm();
    calcSpeedY();
    posMove();

    if (EnBgCheck() & 1) {
        if (isOnEnLiftRemoconTrpln()) {
            // Bounce if on Bouncy Cloud platform
            mSpeed.y = 5.5f;
        } else {
            mSpeed.y = 0.0f;
        }
    }

    if (mAnmChr.getFrame() == 42.0f) {
        mAngle.y = l_base_angleY[mDirection];
        mAngle.x = 0;
        if (EnBgCheck() & 1) {
            mSpeed.y = 2.0f;
        }
    }

    if (mAnmChr.isStop()) {
        changeState(StateID_Walk);
    }

    WaterCheck(mPos, 1.0f);
}
