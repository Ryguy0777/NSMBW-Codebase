#include <kamek.h>
#include <new/bases/d_a_en_block_rotate.hpp>
#include <new/new_profile.hpp>

CUSTOM_ACTOR_PROFILE(EN_BLOCK_ROTATE, daEnBlockRotate_c, fProfile::RIVER_BARREL, fProfile::DRAW_ORDER::RIVER_BARREL, 0x2);

STATE_DEFINE(daEnBlockRotate_c, Wait);
STATE_DEFINE(daEnBlockRotate_c, Flipping);
STATE_DEFINE(daEnBlockRotate_c, Empty);

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

void daEnBlockRotate_c::callBackF(dActor_c *self, dActor_c *other) {
    daEnBlockMain_c::callBackF(self, other);
    daEnBlockRotate_c *_this = (daEnBlockRotate_c *)self;
    if (_this->mContents == 0 && _this->mIndestructible == false) {
        if (other->mKind == STAGE_ACTOR_PLAYER) {
            daPlBase_c *player = (daPlBase_c *)other;
            if (player->isStatus(daPlBase_c::STATUS_2B)) {
                nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(_this->mPos);
                dAudio::g_pSndObjMap->startSound(SE_OBJ_BLOCK_BREAK, soundPos, 0);

                
                dEffActorMng_c::m_instance->createBlockFragEff(_this->mPos, 0x202, -1);

                _this->deleteActor(1);

                player->mSpeed.y = 2.0;
            }
        }
    }
}

int daEnBlockRotate_c::create() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);

    mRes = dResMng_c::m_instance->mRes.getRes("block_rotate", "g3d/block_rotate.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("block_rotate");
    mFlipBlockModel.create(mdl, &mAllocator, 0x227, 1, nullptr);
    dActor_c::setSoftLight_MapObj(mFlipBlockModel);

    mAllocator.adjustFrmHeap();

    Block_CreateClearSet(mPos.y);

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

    mContents = mParam & 0xF;
    mIndestructible = mParam >> 4 & 1;

    changeState(StateID_Wait);

    return SUCCEEDED;
}

int daEnBlockRotate_c::doDelete() {
    if (mStateMgr.getStateID()->isEqual(StateID_Empty)) {
        dPanelObjMgr_c *list = dBg_c::m_bg_p->getPanelObjMgr(0);
        list->removePanelObjList(&mUsedTile);
    }

    mBg.release();
    return SUCCEEDED;
}

int daEnBlockRotate_c::execute() {
    mStateMgr.executeState();
    mBg.calc();
    Block_ExecuteClearSet();

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
    shouldSpawnContinuousStar(&mContents, mPlayerID);
    if (mContents == 14) {
        int isBig = player_bigmario_check(mPlayerID);
        if (isBig) 
            mContents = 1;    
    }
    if (l_early_items[mContents])
        createItem();
}

void daEnBlockRotate_c::initialize_downmove() {
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
    if (mInitialY >= mPos.y)
        blockWasHit(false);
}

void daEnBlockRotate_c::block_downmove() {
    if (mInitialY <= mPos.y)
        blockWasHit(true);
}

void daEnBlockRotate_c::calcModel() {
    if (mStateMgr.getStateID()->operator!=(StateID_Empty)) {
        dActor_c::changePosAngle(&mPos, &mAngle, 1);
        PSMTXTrans(mMatrix, mPos.x, mPos.y + 8.0, mPos.z);

        mMatrix.YrotM(mAngle.y);
        mMatrix.XrotM(mAngle.x);
        mMatrix.ZrotM(mAngle.z);
        
        mFlipBlockModel.setLocalMtx(&mMatrix);
        mFlipBlockModel.setScale(mScale.x, mScale.y, mScale.z);
        mFlipBlockModel.calc(false);
    }

    return;
}

void daEnBlockRotate_c::blockWasHit(bool isDown) {
    mPos.y = mInitialY;

    if (mContents != 0) {
        if (mContents == 10 && mCoinsRemaining > 0) {
            changeState(StateID_Wait);
        } else 
            changeState(StateID_Empty);
    } else 
        changeState(StateID_Flipping);
}

bool daEnBlockRotate_c::playerOverlaps() {
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
    switch (mContents) {
        case 9: // yoshi egg
            if (YoshiEggCreateCheck(0))
                return;
            break;
        case 12: // vine
            item_ivy_set(1, _68D);
            break;
        case 13: // spring
            jumpdai_set();
            break;
        case 10: // 10-coin, don't care for the time mechanic
            mCoinsRemaining--;
        default: // normal items
            dActor_c::construct(fProfile::EN_ITEM, mPlayerID << 16 | (mIsGroundPound * 3) << 18 | l_item_values[mContents] & 0b11111, &mPos, nullptr, mLayer);
            playItemAppearSound(&mPos, l_item_values[mContents], mPlayerID, 0, 0);
            break;
    }
}

void daEnBlockRotate_c::initializeState_Wait() {}

void daEnBlockRotate_c::finalizeState_Wait() {}

void daEnBlockRotate_c::executeState_Wait() {
    int result = ObjBgHitCheck();

    if (result == 0)
        return;

    if (result == 1) {
        changeState(StateID_UpMove);
        mAnotherFlag = 2;
        mIsGroundPound = false;
    } else {
        changeState(StateID_DownMove);
        mAnotherFlag = 1;
        mIsGroundPound = true;
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

void daEnBlockRotate_c::initializeState_Empty() {
    mFlipBlockModel.remove();

    dPanelObjMgr_c *list = dBg_c::m_bg_p->getPanelObjMgr(0);
    list->addPanelObjList(&mUsedTile);

    mUsedTile.mPos.x = mPos.x - 8;
    mUsedTile.mPos.y = -(16 + mPos.y);
    mUsedTile.mTileNumber = 0x32;

    if (!l_early_items[mContents])
        createItem();
}

void daEnBlockRotate_c::finalizeState_Empty() {}

void daEnBlockRotate_c::executeState_Empty() {}