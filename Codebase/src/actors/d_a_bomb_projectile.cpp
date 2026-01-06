#include <kamek.h>
#include <new/bases/d_a_bomb_projectile.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_a_en_bros_base.hpp>
#include <game/bases/d_audio.hpp>
#include <constants/sound_list.h>

void BombProjectileCollcheck(dCc_c *self, dCc_c *other) {
    daBombProjectile_c *bomb = (daBombProjectile_c *)self->mpOwner;
    dActor_c *ac = other->mpOwner;
    u32 otherKind = ac->mKind;
    if (bomb->mPlayerBomb) {
        // If we've been spat by a yoshi
        if (otherKind == dActor_c::STAGE_ACTOR_ENEMY) {
            // Colliding with something
            bomb->mStateMgr.changeState(daBombProjectile_c::StateID_Explode);
        }
    } else {
        // Thrown by a bro
        if (otherKind == dActor_c::STAGE_ACTOR_PLAYER) {
            // Touching player
            bomb->mStateMgr.changeState(daBombProjectile_c::StateID_Explode);
        } else if (otherKind == dActor_c::STAGE_ACTOR_YOSHI) {
            // Touching yoshi
            if (other->mCcData.mAttack == CC_ATTACK_YOSHI_EAT) {
                // If we're colliding with yoshi's tounge
                bomb->mStateMgr.changeState(daBombProjectile_c::StateID_EatWait);
            } else {
                s8 *playerNum = ac->getPlrNo();
                if (*playerNum > -1) {
                    // Yoshi has a player on him
                    bomb->mStateMgr.changeState(daBombProjectile_c::StateID_Explode);
                }
            }
        }
    }
};

void BombExplosionCollcheck(dCc_c *self, dCc_c *other) {
    daBombProjectile_c *bomb = (daBombProjectile_c *)self->mpOwner;
    dActor_c *ac = other->mpOwner;
    if (!bomb->mPlayerBomb) {
        // Only collide with players if we've been thrown by a bro
        u32 otherKind = ac->mKind;
        if (otherKind == dActor_c::STAGE_ACTOR_PLAYER) {
            daPlBase_c *pl = (daPlBase_c *)other;
            pl->setDamage(bomb, daPlBase_c::DAMAGE_NONE);
        } else if (otherKind == dActor_c::STAGE_ACTOR_YOSHI) {
            daYoshi_c *ys = (daYoshi_c *)other;
            s8 *playerNum = ys->getPlrNo();
            if (*playerNum > -1) {
                ys->setDamage(bomb, daPlBase_c::DAMAGE_NONE);
            }
        }
    }
}

CUSTOM_ACTOR_PROFILE(BROS_BOMB, daBombProjectile_c, fProfile::EN_BOMHEI, fProfile::DRAW_ORDER::EN_BOMHEI, 0x22);

STATE_DEFINE(daBombProjectile_c, ThrowWait);
STATE_DEFINE(daBombProjectile_c, EatWait);
STATE_DEFINE(daBombProjectile_c, Throw);
STATE_DEFINE(daBombProjectile_c, Explode);

dCustomProfile_c l_BROS_BOMB_profile(&g_profile_BROS_BOMB, "BROS_BOMB", fProfile::BROS_BOMB);

const sBcSensorPoint l_bombproj_head = { 0, 0x0, 0x10000 };
const sBcSensorLine l_bombproj_foot = { 1, -0x3000, 0x3000, 0 };
const sBcSensorPoint l_bombproj_wall = { 0, 0x6000, 0x6000 };

const sCcDatNewF l_bombproj_cc = {
    {0.0f, 8.0f},
    {8.0f, 8.0f},
    CC_KIND_ENEMY,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) | BIT_FLAG(CC_KIND_YOSHI) |
    BIT_FLAG(CC_KIND_ENEMY) | BIT_FLAG(CC_KIND_TAMA),
    0x8000,
    CC_STATUS_NONE,
    &BombProjectileCollcheck,
};

int daBombProjectile_c::create() {
    // Setup model
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);

    mRes = dResMng_c::m_instance->getRes("bros_bombhei", "g3d/bros_bombhei.brres");
    mResMdl = mRes.GetResMdl("bombhei");
    mModel.create(mResMdl, &mAllocator, 0x108, 1, nullptr);
    setSoftLight_Enemy(mModel);

    mResClr = mRes.GetResAnmClr("bombhei");
	mAnmClr.create(mResMdl, mResClr, &mAllocator, 0, 1);
    mAnmClr.setAnm(mModel, mResClr, 0, m3d::FORWARD_ONCE);
    mModel.setAnm(mAnmClr, 0.0f);

    mAllocator.adjustFrmHeap();

    // Collider
    mCc.set(this, (sCcDatNewF *)&l_bombproj_cc);

    mOwnerID = (fBaseID_e)mParam;

    // mCenterOffs is used to set the "center" of the actor
    // For yoshi tongue and dieFall
    mCenterOffs.set(0.0f, 8.0f, 0.0f);

    // Set size for model culling
    mVisibleAreaSize.set(16.0f, 16.0f);
    mVisibleAreaOffset.set(0.0f, 8.0f);

    // Set yoshi eating behavior
    mEatBehaviour = EAT_TYPE_EAT;

    // Tile sensors
    mBc.set(this, l_bombproj_foot, l_bombproj_head, l_bombproj_wall);

    dActor_c *owner = getParent();

    if (owner->mKind == dActor_c::STAGE_ACTOR_PLAYER) {
        mPlayerBomb = true;
    }

    if (owner) {
        mDirection = owner->mDirection;
    } else {
        mDirection = getPlayerDirection();
    }
    mAngle.y = l_base_angleY[mDirection];

    if (owner->mProfName == fProfile::EN_BOMBBROS) {
        // Wait for the bro to release us
        mStateMgr.changeState(StateID_ThrowWait);
    } else {
        mStateMgr.changeState(StateID_Throw);
    }

    return SUCCEEDED;
}

int daBombProjectile_c::execute() {
    mStateMgr.executeState();
    ActorScrOutCheck(SKIP_NONE);
    return SUCCEEDED;
}

int daBombProjectile_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

void daBombProjectile_c::finalUpdate() {
    // Calculate model
    Mtx someMatrix;
    Mtx thirdMatrix;

    // Do screen wrapping for levels with it enabled
    dActor_c::changePosAngle(&mPos, &mAngle, 1);

    // Set matrix to world position
    PSMTXTrans(mMatrix, mPos.x, mPos.y, mPos.z);

    // Apply rotation vector
    mMatrix.YrotM(mAngle.y);
    mMatrix.XrotM(mAngle.x);
    mMatrix.ZrotM(mAngle.z);
    
    // Apply center offsets
    PSMTXTrans(someMatrix, 0.0f, mCenterOffs.y, 0.0f);
    PSMTXConcat(mMatrix, someMatrix, mMatrix);
    PSMTXTrans(thirdMatrix, 0.0f, -mCenterOffs.y, 0.0f);
    PSMTXConcat(mMatrix, thirdMatrix, mMatrix);

    // Set the matrix for the model
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale.x, mScale.y, mScale.z);
    mModel.calc(false);

    // Store root bone position
    nw4r::g3d::ResNode root = mResMdl.GetResNode("skl_root");
	int rootNodeID = root.GetID();
    mMtx_c rootMtx;
	mModel.getNodeWorldMtxMultVecZero(rootNodeID, mRootPos);

    return;
}

bool daBombProjectile_c::setEatSpitOut(dActor_c *eatingActor) {
    // Coming out of yoshi's mouth
	mDirection = eatingActor->mDirection;
    mPos.y += 3.0f;
    mPos.z = 5750.0f;
    mCc.mCcData.mVsDamage = 0;
    // We own the bomb now
    mPlayerBomb = true;
    mOwnerID = eatingActor->mUniqueID;
	mStateMgr.changeState(StateID_Throw);
	return true;
}

float tileXOffsets[9] = {
    -16.0f, 0.0f, 16.0f,
    -16.0f, 0.0f, 16.0f,
    -16.0f, 0.0f, 16.0f
};

float tileYOffsets[9] = {
    16.0f, 16.0f, 16.0f,
    0.0f, 0.0f, 0.0f,
    -16.0f, -16.0f, -16.0f
};

void daBombProjectile_c::explodeTiles() {
    Vec2 tileToCheck;
    if (mExplosionTimer < 9) {
        tileToCheck.x = (mPos.x * 0.0625f) * 16.0f + tileXOffsets[mExplosionTimer];
        tileToCheck.y = (mPos.y * 0.0625f) * 16.0f + tileYOffsets[mExplosionTimer];
    } else {
        tileToCheck.x = 0.0f;
        tileToCheck.y = 0.0f;
    }
    for (int i = 0; i < 2; i++) {
        u32 type = dBc_c::getUnitType(tileToCheck.x, tileToCheck.y, mLayer);
        if (type & 0x1c) {
            short x, y;
            y = -tileToCheck.y;
            x = tileToCheck.x;
            dBg_c::m_bg_p->BgUnitChange(x, y, mLayer, 0);
        }
    }
}

dActor_c *daBombProjectile_c::getParent() {
    return (dActor_c*)fManager_c::searchBaseByID(mOwnerID);
}

bool daBombProjectile_c::getPlayerDirection() {
    mVec2_c deltaPos;
    dAcPy_c *player = searchNearPlayer(deltaPos);
    return player != nullptr && deltaPos.x < 0.0f;
}

void daBombProjectile_c::initializeState_ThrowWait() {
    mScale.set(0.5f, 0.5f, 0.5f);
}

void daBombProjectile_c::finalizeState_ThrowWait() {
    mScale.set(1.0f, 1.0f, 1.0f);
}

void daBombProjectile_c::executeState_ThrowWait() {
    daEnBrosBase_c *owner = (daEnBrosBase_c *)getParent();
    mVec3_c handPos;
    owner->mMtx1.multVecZero(handPos);
    mPos = handPos;
    mAngle.y = owner->mAngle.y;
    if (mScale.x < 1.0f) {
        mScale.x += 0.02f;
        mScale.y += 0.02f;
        mScale.z += 0.02f;
    }
    if (mReadyToThrow) {
        mStateMgr.changeState(StateID_Throw);
    }
}

void daBombProjectile_c::initializeState_EatWait() {
    mAnmClr.setFrame(0.0f, 0);
    mAnmClr.setRate(0.0f, 0);
    mCc.release();
}

void daBombProjectile_c::finalizeState_EatWait() {}

void daBombProjectile_c::executeState_EatWait() {}

void daBombProjectile_c::initializeState_Throw() {
    mAngle.y = l_base_angleY[mDirection];
    // Now we're adding our physics
    mCc.entry();

    mAnmClr.setRate(1.0f, 0);

    dActor_c *owner = getParent();

    if (!mPlayerBomb) {
        mSpeed.x = (mDirection) ? -2.1f : 2.1f;
        mSpeed.y = 3.5f;
        mSpeedMax.y = -6.0f;
        mAccelY = -0.2f;
        if (owner) {
            mSpeed.x = mSpeed.x + owner->mPosDelta.x * 1.1f;
        }
    } else {
        mSpeed.x = owner->mSpeed.x * 0.5f + ((mDirection) ? -3.85f : 3.85f);
        mSpeed.y = 2.8f;
        mAccelY = -0.225f;
        mSpeedMax.x = (mDirection) ? -2.9f : 2.9f;
        mMaxFallSpeed = -4.0f;
        mSpeedMax.y = -4.0f;
        mAccelF = 0.04f;
    }
}

void daBombProjectile_c::finalizeState_Throw() {}

void daBombProjectile_c::executeState_Throw() {
    mAnmClr.play();
    calcSpeedY();
    calcSpeedX();
    posMove();

    // Effect and sound
    mIgniteEffect.createEffect("Wm_en_bombignition", 0, &mRootPos, nullptr, nullptr);
    mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjEmy->holdSound(SE_EMY_BH_HIBANA, mUniqueID, soundPos, 0);

    // Explode after a while
    if (mAnmClr.isStop(0)) {
        mShouldExplode = true;
    }

    if (mBc.checkFoot() || mBc.checkHead(0) || mBc.checkWall(0)) {
        mShouldExplode = true;
    }

    if (mShouldExplode) {
        changeState(StateID_Explode);
    }
}

const sCcDatNewF l_explode_cc = {
    {0.0f, 8.0f},
    {18.0f, 18.0f},
    CC_KIND_ENEMY,
    CC_ATTACK_SHELL,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) | BIT_FLAG(CC_KIND_YOSHI) |
    BIT_FLAG(CC_KIND_ENEMY) | BIT_FLAG(CC_KIND_TAMA) | BIT_FLAG(CC_KIND_KILLER),
    0,    
    CC_STATUS_NONE,
    &BombExplosionCollcheck,
};

void daBombProjectile_c::initializeState_Explode() {
    mCc.release();
    mEf::createEffect("Wm_en_explosion", 0, &mRootPos, nullptr, nullptr);
    mEf::createEffect("Wm_en_bombheibreak", 0, &mRootPos, nullptr, nullptr);

    mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjEmy->startSound(SE_EMY_BH_BOMB, soundPos, 0);

    mScale.set(0.0f, 0.0f, 0.0f);

    mCc.set(this, (sCcDatNewF *)&l_explode_cc);
    mCc.entry();

    mExplosionTimer = 0;
}

void daBombProjectile_c::finalizeState_Explode() {}

void daBombProjectile_c::executeState_Explode() {
    mExplosionTimer++;
    if (mExplosionTimer < 9) {
        explodeTiles();
        return;
    }
    if (mExplosionTimer != 0x10) {
        return;
    }
    deleteActor(0);
    return;
}