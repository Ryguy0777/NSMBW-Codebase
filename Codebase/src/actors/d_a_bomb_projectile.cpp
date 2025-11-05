#include <kamek.h>
#include <new/bases/d_a_bomb_projectile.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_a_en_bros_base.hpp>
#include <game/bases/d_audio.hpp>
#include <constants/sound_list.h>

void BombProjectileCollcheck(dCc_c *cc1, dCc_c *cc2) {
    daBombProjectile_c *bomb = (daBombProjectile_c *)cc1->mpOwner;
    dActor_c *other = cc2->mpOwner;
    u32 otherKind = other->mKind;
    if (bomb->mPlayerBomb) {
        // If we've been spat by a yoshi
        if (otherKind == dActor_c::STAGE_ACTOR_ENTITY) {
            // Colliding with something
            bomb->mStateMgr.changeState(daBombProjectile_c::StateID_Explode);
        }
    } else {
        // Thrown by a bro
        if (otherKind == dActor_c::STAGE_ACTOR_PLAYER) {
            // Touching player
            bomb->mStateMgr.changeState(daBombProjectile_c::StateID_Explode);
        } else if (otherKind == 2) {
            // Touching yoshi
            if (cc2->mCcData.mAttackCategory == dCc_c::ATTACK_YOSHI_EAT) {
                // If we're colliding with yoshi's tounge
                bomb->mStateMgr.changeState(daBombProjectile_c::StateID_EatWait);
            } else {
                s8 *playerNum = other->getPlrNo();
                if (*playerNum > -1) {
                    // Yoshi has a player on him
                    bomb->mStateMgr.changeState(daBombProjectile_c::StateID_Explode);
                }
            }
        }
    }
};

void BombExplosionCollcheck(dCc_c *cc1, dCc_c *cc2) {
    daBombProjectile_c *bomb = (daBombProjectile_c *)cc1->mpOwner;
    dActor_c *other = cc2->mpOwner;
    if (!bomb->mPlayerBomb) {
        // Only collide with players if we've been thrown by a bro
        u32 otherKind = other->mKind;
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

dCustomProfile_c bombProjectileProfile(&g_profile_BROS_BOMB, "BROS_BOMB", fProfile::BROS_BOMB);

STATE_DEFINE(daBombProjectile_c, ThrowWait);
STATE_DEFINE(daBombProjectile_c, EatWait);
STATE_DEFINE(daBombProjectile_c, Throw);
STATE_DEFINE(daBombProjectile_c, Explode);

sCcDatNewF l_bombproj_cc = {
    0.0,                        // mOffsetX
    8.0,                        // mOffsetY
    8.0,                        // mWidth
    8.0,                        // mHeight
    dCc_c::CAT_ENTITY,          // mCategory
    0,                          // mAttackCategory
    0x4F,                       // mCategoryInteract
    0x8000,                     // mAttackCategoryInteract     
    0,                          // mFlag
    &BombProjectileCollcheck,   // mCallback
};

int daBombProjectile_c::create() {
    // Setup model
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);

    mRes = dResMng_c::m_instance->mRes.getRes("bros_bombhei", "g3d/bros_bombhei.brres");
    mResMdl = mRes.GetResMdl("bombhei");
    mModel.create(mResMdl, &mAllocator, 0x108, 1, nullptr);
    dActor_c::setSoftLight_Enemy(mModel);

    mResClr = mRes.GetResAnmClr("bombhei");
	mAnmClr.create(mResMdl, mResClr, &mAllocator, 0, 1);
    mAnmClr.setAnm(mModel, mResClr, 0, m3d::FORWARD_ONCE);
    mModel.setAnm(mAnmClr, 0.0f);

    mAllocator.adjustFrmHeap();

    // Collider
    mCc.set(this, &l_bombproj_cc);

    mOwnerID = (fBaseID_e)mParam;

    // mCenterOffs is used to set the "center" of the actor
    // For yoshi tongue and dieFall
    mCenterOffs = mVec3_c(0.0, 8.0, 0.0);

    // Set size for model culling
    mVisibleAreaSize.x = 16.0;
    mVisibleAreaSize.y = 16.0;
    mVisibleAreaOffset.x = 0.0;
    mVisibleAreaOffset.y = 8.0;

    // Set yoshi eating behavior
    mEatBehaviour = EAT_TYPE_EAT;

    // Tile collider
    static const dBcSensorLine_c below(-4<<12, 4<<12, 0<<12);
	static const dBcSensorPoint_c above(0<<12, 16<<12);
    static const dBcSensorLine_c adjacent(6<<12, 8<<12, 7<<12);

	mBc.set(this, (dBcSensor_c*)&below, (dBcSensor_c*)&above, (dBcSensor_c*)&adjacent);

    dActor_c *owner = getParent();

    if (owner->mKind == dActor_c::STAGE_ACTOR_PLAYER) {
        mPlayerBomb = true;
    }

    if (owner) {
        mDirection = owner->mDirection;
    } else {
        mDirection = getPlayerDirection();
    }
    mAngle.y = (mDirection) ? 0xE000 : 0x2000;

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
    ActorScrOutCheck(0);
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
    PSMTXTrans(someMatrix, 0.0, mCenterOffs.y, 0.0);
    PSMTXConcat(mMatrix, someMatrix, mMatrix);
    PSMTXTrans(thirdMatrix, 0.0, -mCenterOffs.y, 0.0);
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
    mPos.y += 3.0;
    mPos.z = 5750.0;
    mCc.mCcData.mAttackCategoryInteract = 0;
    // We own the bomb now
    mPlayerBomb = true;
    mOwnerID = eatingActor->mUniqueID;
	mStateMgr.changeState(StateID_Throw);
	return true;
}

float tileXOffsets[9] = {
    -16.0, 0.0, 16.0,
    -16.0, 0.0, 16.0,
    -16.0, 0.0, 16.0
};

float tileYOffsets[9] = {
    16.0, 16.0, 16.0,
    0.0, 0.0, 0.0,
    -16.0, -16.0, -16.0
};

void daBombProjectile_c::explodeTiles() {
    Vec2 tileToCheck;
    if (mExplosionTimer < 9) {
        tileToCheck.x = (mPos.x * 0.0625) * 16.0 + tileXOffsets[mExplosionTimer];
        tileToCheck.y = (mPos.y * 0.0625) * 16.0 + tileYOffsets[mExplosionTimer];
    } else {
        tileToCheck.x = 0.0;
        tileToCheck.y = 0.0;
    }
    for (int i = 0; i < 2; i++) {
        u32 type = dBc_c::getUnitType(tileToCheck.x, tileToCheck.y, mLayer);
        if (type & (UnitType::QuestionBlock|UnitType::ExplodableBlock|UnitType::BreakableBlock)) {
            dBg_c::m_bg_p->BgUnitChange((int)tileToCheck.x & 0xffff, (int)-tileToCheck.y & 0xffff, mLayer, 0);
        }
    }
}

dActor_c *daBombProjectile_c::getParent() {
    return (dActor_c*)fManager_c::searchBaseByID(mOwnerID);
}

bool daBombProjectile_c::getPlayerDirection() {
    mVec2_c deltaPos;
    dAcPy_c *player = searchNearPlayer(deltaPos);
    return player != nullptr && deltaPos.x < 0.0;
}

void daBombProjectile_c::initializeState_ThrowWait() {
    mScale = mVec3_c(0.5, 0.5, 0.5);
}

void daBombProjectile_c::finalizeState_ThrowWait() {
    mScale = mVec3_c(1.0, 1.0, 1.0);
}

void daBombProjectile_c::executeState_ThrowWait() {
    daEnBrosBase_c *owner = (daEnBrosBase_c *)getParent();
    mVec3_c handPos;
    owner->mMtx1.multVecZero(handPos);
    mPos = handPos;
    mAngle.y = owner->mAngle.y;
    if (mScale.x < 1.0) {
        mScale.x += 0.02;
        mScale.y += 0.02;
        mScale.z += 0.02;
    }
    if (mReadyToThrow) {
        mStateMgr.changeState(StateID_Throw);
    }
}

void daBombProjectile_c::initializeState_EatWait() {
    mAnmClr.setFrame(0.0, 0);
    mAnmClr.setRate(0.0, 0);
    mCc.release();
}

void daBombProjectile_c::finalizeState_EatWait() {}

void daBombProjectile_c::executeState_EatWait() {}

void daBombProjectile_c::initializeState_Throw() {
    mAngle.y = (mDirection) ? 0xE000 : 0x2000;
    // Now we're adding our physics
    mCc.entry();

    mAnmClr.setRate(1.0, 0);

    dActor_c *owner = getParent();

    if (!mPlayerBomb) {
        mSpeed.x = (mDirection) ? -2.1 : 2.1;
        mSpeed.y = 3.5;
        mSpeedMax.y = -6.0;
        mAccelY = -0.2;
        if (owner) {
            mSpeed.x = mSpeed.x + owner->mPosDelta.x * 1.1;
        }
    } else {
        mSpeed.x = owner->mSpeed.x * 0.5 + ((mDirection) ? -3.85 : 3.85);
        mSpeed.y = 2.8;
        mAccelY = -0.225;
        mSpeedMax.x = (mDirection) ? -2.9: 2.9;
        mMaxFallSpeed = -4.0;
        mSpeedMax.y = -4.0;
        mAccelF = 0.04;
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
        mStateMgr.changeState(StateID_Explode);
    }
}

sCcDatNewF l_explode_cc = {
    0.0,                        // mOffsetX
    8.0,                        // mOffsetY
    18.0,                       // mWidth
    18.0,                       // mHeight
    dCc_c::CAT_ENTITY,          // mCategory
    dCc_c::ATTACK_SHELL,        // mAttackCategory
    0xEF,                       // mCategoryInteract
    0,                          // mAttackCategoryInteract     
    1,                          // mFlag
    &BombExplosionCollcheck,    // mCallback
};

void daBombProjectile_c::initializeState_Explode() {
    mCc.release();
    mEf::createEffect("Wm_en_explosion", 0, &mRootPos, nullptr, nullptr);
    mEf::createEffect("Wm_en_bombheibreak", 0, &mRootPos, nullptr, nullptr);

    mVec2_c soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjEmy->startSound(SE_EMY_BH_BOMB, soundPos, 0);
    
    mScale = mVec3_c(0.0, 0.0, 0.0);

    mCc.set(this, &l_explode_cc);
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