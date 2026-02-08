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

const char* l_BLOCK_ROTATE_res[] = {"block_rotate", NULL};
const dActorData_c c_BLOCK_ROTATE_actor_data = {fProfile::EN_BLOCK_ROTATE, 8, -16, 8, 0, 16, 16, 0, 0, 0, 0, ACTOR_CREATE_MAPOBJ};
dCustomProfile_c l_BLOCK_ROTATE_profile(&g_profile_EN_BLOCK_ROTATE, "EN_BLOCK_ROTATE", SpriteId::EN_BLOCK_ROTATE, &c_BLOCK_ROTATE_actor_data, l_BLOCK_ROTATE_res);

sBgSetInfo l_flipblock_bgc_info = {
    mVec2_c(-8, 16),
    mVec2_c(8, 0),
    &daEnBlockRotate_c::callBackF,
    &daEnBlockMain_c::callBackH,
    &daEnBlockMain_c::callBackW
};

// We have our own callbackF function to detect player spinjumps

void daEnBlockRotate_c::callBackF(dActor_c *self, dActor_c *other) {
    // Call OG function
    daEnBlockMain_c::callBackF(self, other);
    daEnBlockRotate_c *this_ = (daEnBlockRotate_c *)self;
    // Only break if empty and indestructible
    if (this_->mContents == 0 && this_->mIndestructible == false) {
        if (other->mKind == STAGE_ACTOR_PLAYER) {
            daPlBase_c *player = (daPlBase_c *)other;
            // Statuses 0xA9 and 0x2B are only both set when spinjumping
            if (player->isStatus(0xA9) && player->isStatus(daPlBase_c::STATUS_2B)) {
                if (player->mPowerup != POWERUP_NONE && player->mPowerup != POWERUP_MINI_MUSHROOM) {
                    this_->destroyBlock();
                    // Move player upwards slightly
                    float playerSpeedInc;
                    if (player->mKey.buttonTwo()) {
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
    // Setup model
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);

    mRes = dResMng_c::m_instance->getRes("block_rotate", "g3d/block_rotate.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("block_rotate");
    mFlipBlockModel.create(mdl, &mAllocator, 0x227, 1, nullptr);
    setSoftLight_MapObj(mFlipBlockModel);

    mAllocator.adjustFrmHeap();

    Block_CreateClearSet(mPos.y);

    // Set collider
    mBg.set(this, &l_flipblock_bgc_info, 3, mLayer, nullptr);
    mBg.mFlags = 0x260;

    mBg.mBelowCheckFunc = &daEnBlockMain_c::checkRevFoot;
    mBg.mAboveCheckFunc = &daEnBlockMain_c::checkRevHead;
    mBg.mAdjCheckFunc = &daEnBlockMain_c::checkRevWall;

    mBg.mBelowCallback = &daEnBlockRotate_c::callBackF;
    mBg.mAboveCallback = &daEnBlockMain_c::callBackH;
    mBg.mAdjCallback = &daEnBlockMain_c::callBackW;

    mBg.entry();

    mCoinsRemaining = 10;

    // Sprite settings
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

    // Only delete if not flipping
    if (mStateMgr.getStateID()->isEqual(StateID_Wait)) {
        ActorScrOutCheck(SKIP_NONE);
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
    continue_star_check(&mContents, mPlayerID);
    // Handle mushroom-if-small
    if (mContents == 14) {
        int isBig = player_bigmario_check(mPlayerID);
        if (isBig) {
            mContents = 1;
        }
    }
    // Create coin items/propeller on block hit
    if (l_early_items[mContents]) {
        createItem();
    }
}

void daEnBlockRotate_c::initialize_downmove() {
    // Same as upmove
    continue_star_check(&mContents, mPlayerID);
    if (mContents == 14) {
        int isBig = player_bigmario_check(mPlayerID);
        if (isBig) {
            mContents = 1;
        } 
    }
    if (l_early_items[mContents]) {
        createItem();
    }
}

void daEnBlockRotate_c::block_upmove() {
    // Call blockWasHit at the end of upmove
    if (mInitialY >= mPos.y) {
        blockWasHit(false);
    }
}

void daEnBlockRotate_c::block_downmove() {
    // Call blockWasHit at the end of downmove
    if (mInitialY <= mPos.y) {
        blockWasHit(true);
    }
}

void daEnBlockRotate_c::calcModel() {
    // Calculate model
    changePosAngle(&mPos, &mAngle, 1);
    PSMTXTrans(mMatrix, mPos.x, mPos.y + 8.0, mPos.z);

    mMatrix.YrotM(mAngle.y);
    mMatrix.XrotM(mAngle.x);
    mMatrix.ZrotM(mAngle.z);
    
    mFlipBlockModel.setLocalMtx(&mMatrix);
    mFlipBlockModel.setScale(mScale);
    mFlipBlockModel.calc(false);

    return;
}

void daEnBlockRotate_c::blockWasHit(bool isDown) {
    // Handle state changes after hitting block
    mPos.y = mInitialY;

    if (mContents != 0) {
        // We've already spawned our coin if we're a 10-coin block, so go back to wait
        if (mContents == 10 && mCoinsRemaining > 0) {
            changeState(StateID_Wait);
        } else 
            createEmpty();
    } else {
        changeState(StateID_Flipping);
    }
}

bool daEnBlockRotate_c::playerOverlaps() {
    // Check if we're overlapping with a player
    // Used during flip state
    dActor_c *player = nullptr;

    mVec3_c myBL(mPos.x - 8.0f, mPos.y - 8.0f, 0.0f);
    mVec3_c myTR(mPos.x + 8.0f, mPos.y + 8.0f, 0.0f);

    while ((player = (dActor_c*)fManager_c::searchBaseByProfName(fProfile::PLAYER, player)) != 0) {
        float centerX = player->mPos.x + player->mCc.mCcData.mOffset.x;
        float centerY = player->mPos.y + player->mCc.mCcData.mOffset.y;

        float left = centerX - player->mCc.mCcData.mSize.x;
        float right = centerX + player->mCc.mCcData.mSize.x;

        float top = centerY + player->mCc.mCcData.mSize.y;
        float bottom = centerY - player->mCc.mCcData.mSize.y;

        mVec3_c playerBL(left, bottom + 0.1f, 0.0f);
        mVec3_c playerTR(right, top - 0.1f, 0.0f);

        if (dGameCom::checkRectangleOverlap(&playerBL, &playerTR, &myBL, &myTR, 0.0f)) {
            return true;
        }
    }

    return false;
}

void daEnBlockRotate_c::createItem() {
    // Spawn block contents
    switch (mContents) {
        case 9: // Yoshi egg
            if (YoshiEggCreateCheck(0))
                return;
            break;
        case 12: // Vine
            item_ivy_set(1, m_169);
            break;
        case 13: // Spring
            jumpdai_set();
            break;
        case 10: // 10-coin, don't care for the time mechanic
            mCoinsRemaining--;
        default: // Normal items
            dActor_c::construct(fProfile::EN_ITEM, mPlayerID << 16 | (mIsGroundPound * 3) << 18 | l_item_values[mContents] & 0b11111, &mPos, nullptr, mLayer);
            // Play item spawn sound
            item_sound_set(mPos, l_item_values[mContents], mPlayerID, 0, 0);
            break;
    }
}

void daEnBlockRotate_c::createEmpty() {
    // Delete block
    deleteActor(1);
    
    // Create empty block tile
    u16 worldX = ((u16)mPos.x) & 0xFFF0;
	u16 worldY = ((u16)-(mPos.y + 16.0)) & 0xFFF0;

    dBg_c::m_bg_p->BgUnitChange(worldX, worldY, mLayer, 0x0001);

    // Spawn item if we haven't already
    if (!l_early_items[mContents]) {
        createItem();
    }
}

void daEnBlockRotate_c::destroyBlock() {
    // Delete block
    deleteActor(1);

    // Play break sound and spawn shard effect
    nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjMap->startSound(SE_OBJ_BLOCK_BREAK, soundPos, 0);

    dEffActorMng_c::m_instance->createBlockFragEff(mPos, 0x903, -1);
}

void daEnBlockRotate_c::initializeState_Wait() {}

void daEnBlockRotate_c::finalizeState_Wait() {}

void daEnBlockRotate_c::executeState_Wait() {
    // Check if the block has been hit
    int result = ObjBgHitCheck();

    if (result == 1) {
        // Hit from below
        mAnotherFlag = 2;
        mIsGroundPound = false;
        changeState(StateID_UpMove);
    } else if (result == 2) {
        // Hit from above
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
    mBg.set(this, &l_flipblock_bgc_info, 3, mLayer, nullptr);
    mBg.entry();
}

void daEnBlockRotate_c::executeState_Flipping() {
    if (mIsGroundPound) {
        mAngle.x += 0x800;
    } else {
        mAngle.x -= 0x800;
    }

    if (mAngle.x == 0) {
        mFlipsRemaining--;
        if (mFlipsRemaining <= 0) {
            if (!playerOverlaps()) {
                changeState(StateID_Wait);
            }
        }
    }
}

// Add new Flip Block fragment tiles to dEffBlockFrag_c
u16 newTableForEffBlockFrag[10][4] = {
    {0x40, 0x41, 0x42, 0x40}, // Brick Block
    {0x4C, 0x4D, 0x4E, 0x4C}, // Stone Block
    {0x49, 0x4A, 0x4B, 0x49}, // Wood Block
    {0x43, 0x44, 0x45, 0x43}, // ?-Block
    {0x46, 0x47, 0x48, 0x46}, // Used Block
    {0x50, 0x51, 0x52, 0x50}, // Red Block
    {0x46, 0x47, 0x48, 0x46}, // Used Block
    {0x4F, 0x4F, 0x4F, 0x4F}, // Empty Tile (unused?)
    {0x2A0, 0x2A2, 0x2A2, 0x2A0}, // Final Battle
    {0x53, 0x54, 0x55, 0x53}, // Flip Block (new)
};

// Remove block type validity check
kmWriteNop(0x800909e0);

// Insert new table
kmBranchDefAsm(0x80090af0, 0x80090af4) {
    lis r3, newTableForEffBlockFrag@h
    ori r3, r3, newTableForEffBlockFrag@l
    add r21, r3, r0
    blr
}