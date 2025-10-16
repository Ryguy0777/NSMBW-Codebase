#include <kamek.h>
#include <new/bases/d_a_en_block_message.hpp>
#include <new/new_profile.hpp>
#include <new/bases/d_msgbox_window.hpp>

CUSTOM_ACTOR_PROFILE(EN_BLOCK_MESSAGE, daEnBlockMessage_c, fProfile::EN_BLOCK, fProfile::DRAW_ORDER::EN_BLOCK, 0);

STATE_DEFINE(daEnBlockMessage_c, Wait);

const SpriteData msgblockSpriteData = {fProfile::EN_BLOCK_MESSAGE, 8, -8, 8, -8, 16, 16, 0, 0, 0, 0, 0x8};
dCustomProfile_c msgblockProfile(&g_profile_EN_BLOCK_MESSAGE, "EN_BLOCK_MESSAGE", SpriteId::EN_BLOCK_MESSAGE, &msgblockSpriteData);

sBgSetInfo l_msgblock_info = {
    mVec2_c(-8, 8),
    mVec2_c(8, -8),
    &daEnBlockMain_c::callBackF,
    &daEnBlockMain_c::callBackH,
    &daEnBlockMain_c::callBackW
};

int daEnBlockMessage_c::create() {
    Block_CreateClearSet(mPos.y);

    // set collider
    mBg.set(this, &l_msgblock_info, 3, mLayer, nullptr);
    mBg.mFlags = 0x260;

    mBg.mBelowCallback = &daEnBlockMain_c::checkRevFoot;
    mBg.mAboveCallback = &daEnBlockMain_c::checkRevHead;
    mBg.mAdjCallback = &daEnBlockMain_c::checkRevWall;

    mBg.mBelowCheckFunc = &daEnBlockMain_c::callBackF;
    mBg.mAboveCheckFunc = &daEnBlockMain_c::callBackH;
    mBg.mAdjCheckFunc = &daEnBlockMain_c::callBackW;

    mBg.entry();

    // setup tile renderer
    dPanelObjMgr_c *list = dBg_c::m_bg_p->getPanelObjMgr(0);
    list->addPanelObjList(&mTile);

    mTile.mPos.x = mPos.x - 8;
    mTile.mPos.y = -(8 + mPos.y);
    mTile.mTileNumber = 0x9A;

    // sprite settings
    mMessageId = mParam & 0xFF;

    changeState(StateID_Wait);

    return SUCCEEDED;
}


int daEnBlockMessage_c::doDelete() {
    // remove tile renderer and collider
    dPanelObjMgr_c *list = dBg_c::m_bg_p->getPanelObjMgr(0);
    list->removePanelObjList(&mTile);

    mBg.release();
    return SUCCEEDED;
}


int daEnBlockMessage_c::execute() {
    mStateMgr.executeState();
    mBg.calc();
    Block_ExecuteClearSet();

    // update tile collider
    mTile.setPos(mPos.x-8, -(8+mPos.y), mPos.z);
    mTile.setScaleFoot(mScale.x);

    if (mStateMgr.getStateID()->isEqual(StateID_Wait)) {
        ActorScrOutCheck(0);
    }

    return true;
}

void daEnBlockMessage_c::block_upmove() {
    if (mInitialY >= mPos.y)
        blockWasHit(false);
}

void daEnBlockMessage_c::block_downmove() {
    if (mInitialY <= mPos.y)
        blockWasHit(true);
}

void daEnBlockMessage_c::blockWasHit(bool isDown) {
    mPos.y = mInitialY;

    dMsgBoxWindow_c::m_instance->showMessage(mMessageId);

    mBg.set(this, &l_msgblock_info, 3, mLayer, nullptr);
    mBg.entry();
    
    changeState(StateID_Wait);
}

void daEnBlockMessage_c::initializeState_Wait() {}

void daEnBlockMessage_c::finalizeState_Wait() {}

void daEnBlockMessage_c::executeState_Wait() {
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