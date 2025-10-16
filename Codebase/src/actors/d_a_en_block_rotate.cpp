#include <kamek.h>
#include <new/bases/d_a_en_block_rotate.hpp>
#include <new/new_profile.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <constants/sound_list.h>
#include <game/bases/d_eff_actor_manager.hpp>


CUSTOM_ACTOR_PROFILE(EN_BLOCK_ROTATE, daEnBlockRotate_c, fProfile::RIVER_BARREL, fProfile::DRAW_ORDER::RIVER_BARREL, 0x2);

STATE_DEFINE(daEnBlockRotate_c, Wait);
STATE_DEFINE(daEnBlockRotate_c, Flipping);

const char* flipblockArcList[] = {"block_rotate", NULL};
const SpriteData flipblockSpriteData = {fProfile::EN_BLOCK_ROTATE, 8, -16, 8, 0, 16, 16, 0, 0, 0, 0, 0x8};
dCustomProfile_c flipblockProfile(&g_profile_EN_BLOCK_ROTATE, "EN_BLOCK_ROTATE", SpriteId::EN_BLOCK_ROTATE, &flipblockSpriteData, flipblockArcList);

sBgSetInfo l_flipblock_info = {
    mVec2_c(-8, 16),
    mVec2_c(8, 0),
    &daEnBlockRotate_c::callBackF,
    &daEnBlockMain_c::callBackH,
    &daEnBlockMain_c::callBackW
};

// we have our own callbackF function to detect player spinjumps

void daEnBlockRotate_c::callBackF(dActor_c *self, dActor_c *other) {
    // call OG function
    daEnBlockMain_c::callBackF(self, other);
    daEnBlockRotate_c *_this = (daEnBlockRotate_c *)self;
    // only break if empty and indestructible
    if (_this->mContents == 0 && _this->mIndestructible == false) {
        if (other->mKind == STAGE_ACTOR_PLAYER) {
            daPlBase_c *player = (daPlBase_c *)other;
            // statuses 0xA9 and 0x2B are only both set when spinjumping
            if (player->isStatus(0xA9) && player->isStatus(daPlBase_c::STATUS_2B)) {
                if (player->mPowerup != POWERUP_NONE && player->mPowerup != POWERUP_MINI_MUSHROOM) {
                    _this->destroyBlock();
                    // move player upwards slightly
                    float playerSpeedInc;
                    if (player->mKey.buttonJump()) {
                        playerSpeedInc = 3.0;
                    } else {
                        playerSpeedInc = 2.5;
                    }
                    player->mSpeed.y = playerSpeedInc;
                }
            }
        }
    }
}

int daEnBlockRotate_c::create() {
    // setup model
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);

    mRes = dResMng_c::m_instance->mRes.getRes("block_rotate", "g3d/block_rotate.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("block_rotate");
    mFlipBlockModel.create(mdl, &mAllocator, 0x227, 1, nullptr);
    dActor_c::setSoftLight_MapObj(mFlipBlockModel);

    mAllocator.adjustFrmHeap();

    Block_CreateClearSet(mPos.y);

    // set collider
    mBg.set(this, &l_flipblock_info, 3, mLayer, nullptr);
    mBg.mFlags = 0x260;

    mBg.mBelowCallback = &daEnBlockMain_c::checkRevFoot;
    mBg.mAboveCallback = &daEnBlockMain_c::checkRevHead;
    mBg.mAdjCallback = &daEnBlockMain_c::checkRevWall;

    mBg.mBelowCheckFunc = &daEnBlockRotate_c::callBackF;
    mBg.mAboveCheckFunc = &daEnBlockMain_c::callBackH;
    mBg.mAdjCheckFunc = &daEnBlockMain_c::callBackW;

    mBg.entry();

    mCoinsRemaining = 10;

    // sprite settings
    mContents = mParam & 0xF;
    mIndestructible = mParam >> 4 & 1;

    changeState(StateID_Wait);

    return SUCCEEDED;
}

int daEnBlockRotate_c::doDelete() {
    mBg.release();
    return SUCCEEDED;
}

int daEnBlockRotate_c::execute() {
    mStateMgr.executeState();
    mBg.calc();
    Block_ExecuteClearSet();

    // only delete if not flipping
    if (mStateMgr.getStateID()->isEqual(StateID_Wait)) {
        ActorScrOutCheck(0);
    }

    return SUCCEEDED;
}

int daEnBlockRotate_c::draw() {
    mFlipBlockModel.entry();
    return SUCCEEDED;
}

int daEnBlockRotate_c::preDraw() {
    int ret = dActor_c::preDraw();
    if (ret) {
        calcModel();
    }
    return ret;
}

void daEnBlockRotate_c::initialize_upmove() {
    // shouldSpawnContinuousStar sets the contents to either 7 (star) or 1 (coin)
    shouldSpawnContinuousStar(&mContents, mPlayerID);
    // handle mushroom-if-small
    if (mContents == 14) {
        int isBig = player_bigmario_check(mPlayerID);
        if (isBig) 
            mContents = 1;    
    }
    // create coin items/propeller on block hit
    if (l_early_items[mContents])
        createItem();
}

void daEnBlockRotate_c::initialize_downmove() {
    // same as upmove
    shouldSpawnContinuousStar(&mContents, mPlayerID);
    if (mContents == 14) {
        int isBig = player_bigmario_check(mPlayerID);
        if (isBig) 
            mContents = 1;    
    }
    if (l_early_items[mContents])
        createItem();
}

void daEnBlockRotate_c::block_upmove() {
    // call blockWasHit at the end of upmove
    if (mInitialY >= mPos.y)
        blockWasHit(false);
}

void daEnBlockRotate_c::block_downmove() {
    // call blockWasHit at the end of downmove
    if (mInitialY <= mPos.y)
        blockWasHit(true);
}

void daEnBlockRotate_c::calcModel() {
    // calculate model
    dActor_c::changePosAngle(&mPos, &mAngle, 1);
    PSMTXTrans(mMatrix, mPos.x, mPos.y + 8.0, mPos.z);

    mMatrix.YrotM(mAngle.y);
    mMatrix.XrotM(mAngle.x);
    mMatrix.ZrotM(mAngle.z);
    
    mFlipBlockModel.setLocalMtx(&mMatrix);
    mFlipBlockModel.setScale(mScale.x, mScale.y, mScale.z);
    mFlipBlockModel.calc(false);

    return;
}

void daEnBlockRotate_c::blockWasHit(bool isDown) {
    // handle state changes after hitting block
    mPos.y = mInitialY;

    if (mContents != 0) {
        // we've already spawned our coin if we're a 10-coin block, so go back to wait
        if (mContents == 10 && mCoinsRemaining > 0) {
            changeState(StateID_Wait);
        } else 
            createEmpty();
    } else 
        changeState(StateID_Flipping);
}

bool daEnBlockRotate_c::playerOverlaps() {
    // check if we're overlapping with a player
    // used during flip state
    dActor_c *player = nullptr;

    mVec3_c myBL(mPos.x - 8.0f, mPos.y - 8.0f, 0.0f);
    mVec3_c myTR(mPos.x + 8.0f, mPos.y + 8.0f, 0.0f);

    while ((player = (dActor_c*)fManager_c::searchBaseByProfName(fProfile::PLAYER, player)) != 0) {
        float centerX = player->mPos.x + player->mCc.mCcData.mOffsetX;
        float centerY = player->mPos.y + player->mCc.mCcData.mOffsetY;

        float left = centerX - player->mCc.mCcData.mWidth;
        float right = centerX + player->mCc.mCcData.mWidth;

        float top = centerY + player->mCc.mCcData.mHeight;
        float bottom = centerY - player->mCc.mCcData.mHeight;

        mVec3_c playerBL(left, bottom + 0.1f, 0.0f);
        mVec3_c playerTR(right, top - 0.1f, 0.0f);

        if (dGameCom::checkRectangleOverlap(&playerBL, &playerTR, &myBL, &myTR, 0.0))
            return true;
    }

    return false;
}

void daEnBlockRotate_c::createItem() {
    // spawn block contents
    switch (mContents) {
        case 9: // yoshi egg
            if (YoshiEggCreateCheck(0))
                return;
            break;
        case 12: // vine
            item_ivy_set(1, m_169);
            break;
        case 13: // spring
            jumpdai_set();
            break;
        case 10: // 10-coin, don't care for the time mechanic
            mCoinsRemaining--;
        default: // normal items
            dActor_c::construct(fProfile::EN_ITEM, mPlayerID << 16 | (mIsGroundPound * 3) << 18 | l_item_values[mContents] & 0b11111, &mPos, nullptr, mLayer);
            // play item spawn sound
            playItemAppearSound(&mPos, l_item_values[mContents], mPlayerID, 0, 0);
            break;
    }
}

void daEnBlockRotate_c::createEmpty() {
    // delete block
    deleteActor(1);
    
    // create empty block tile
    u16 worldX = ((u16)mPos.x) & 0xFFF0;
	u16 worldY = ((u16)-(mPos.y + 16.0)) & 0xFFF0;

    dBg_c::m_bg_p->BgUnitChange(worldX, worldY, mLayer, 0x0001);

    // spawn item if we haven't already
    if (!l_early_items[mContents])
        createItem();
}

void daEnBlockRotate_c::destroyBlock() {
    // delete block
    deleteActor(1);

    // play break sound and spawn shard effect
    nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjMap->startSound(SE_OBJ_BLOCK_BREAK, soundPos, 0);

    dEffActorMng_c::m_instance->createBlockFragEff(mPos, 0x202, -1);
}

void daEnBlockRotate_c::initializeState_Wait() {}

void daEnBlockRotate_c::finalizeState_Wait() {}

void daEnBlockRotate_c::executeState_Wait() {
    // check if the block has been hit
    int result = ObjBgHitCheck();

    if (result == 1) {
        // hit from below
        mAnotherFlag = 2;
        mIsGroundPound = false;
        changeState(StateID_UpMove);
    } else if (result == 2) {
        // hit from above
        mAnotherFlag = 1;
        mIsGroundPound = true;
        changeState(StateID_DownMove);
    }
}


void daEnBlockRotate_c::initializeState_Flipping() {
    mFlipsRemaining = 7;
    mBg.release();
}

void daEnBlockRotate_c::finalizeState_Flipping() {
    mBg.set(this, &l_flipblock_info, 3, mLayer, nullptr);
    mBg.entry();
}

void daEnBlockRotate_c::executeState_Flipping() {
    if (mIsGroundPound)
        mAngle.x += 0x800;
    else
        mAngle.x -= 0x800;

    if (mAngle.x == 0) {
        mFlipsRemaining--;
        if (mFlipsRemaining <= 0) {
            if (!playerOverlaps())
                changeState(StateID_Wait);
        }
    }
}