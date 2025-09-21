#include <kamek.h>
#include <new/bases/d_a_en_block_flag.hpp>
#include <new/new_profile.hpp>

CUSTOM_ACTOR_PROFILE(EN_BLOCK_FLAG, daEnBlockFlag_c, fProfile::EN_OBJ_HATENAB_BLOCK, fProfile::DRAW_ORDER::OBJ_PIPE_UP, 0);

STATE_DEFINE(daEnBlockFlag_c, Wait);

const SpriteData eventblockSpriteData = {fProfile::EN_BLOCK_FLAG, 8, -8, 8, -8, 16, 16, 0, 0, 0, 0, 0x8};
dCustomProfile_c eventblockProfile(&g_profile_EN_BLOCK_FLAG, "EN_BLOCK_FLAG", SpriteId::EN_BLOCK_FLAG, &eventblockSpriteData);

sBgSetInfo l_eventblock_info = {
    mVec2_c(-8, 8),
    mVec2_c(8, -8),
    &daEnBlockMain_c::callBackF,
    &daEnBlockMain_c::callBackH,
    &daEnBlockMain_c::callBackW
};

int daEnBlockFlag_c::create() {
    Block_CreateClearSet(mPos.y);

    // set collider
    mBg.set(this, &l_eventblock_info, 3, mLayer, nullptr);
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
    mTile.mTileNumber = 0x9C;

    // sprite settings
    mFlagMode = (BLOCK_FLAG_MODE_e)(mParam & 1);

    changeState(StateID_Wait);

    return SUCCEEDED;
}


int daEnBlockFlag_c::doDelete() {
    // remove tile renderer and collider
    dPanelObjMgr_c *list = dBg_c::m_bg_p->getPanelObjMgr(0);
    list->removePanelObjList(&mTile);

    mBg.release();
    return SUCCEEDED;
}


int daEnBlockFlag_c::execute() {
    mStateMgr.executeState();
    mBg.calc();
    Block_ExecuteClearSet();

    // assign event IDs
    // done here instead of create because mEventNums aren't set during creation
    mEventID1 = (mEventNums >> 8 & 0xFF)-1;
    mEventID2 = (mEventNums & 0xFF)-1;

    // update tile collider
    mTile.setPos(mPos.x-8, -(8+mPos.y), mPos.z);
    mTile.setScaleFoot(mScale.x);

    equaliseEvents();

    // update tile number based on event id 1
    bool isActive = checkEvent(mEventID1);

    mTile.mTileNumber = (isActive ? 0x9B : 0x9C);

    if (mStateMgr.getStateID()->isEqual(StateID_Wait)) {
        ActorScrOutCheck(0);
    }

    return true;
}

// play sound upon being hit
void daEnBlockFlag_c::initialize_upmove() {
    nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjMap->startSound(SE_OBJ_STEP_ON_SWITCH, soundPos, 0);
}

void daEnBlockFlag_c::initialize_downmove() {
    nw4r::math::VEC2 soundPos = dAudio::cvtSndObjctPos(mPos);
    dAudio::g_pSndObjMap->startSound(SE_OBJ_STEP_ON_SWITCH, soundPos, 0);
}

void daEnBlockFlag_c::block_upmove() {
    if (mInitialY >= mPos.y)
        blockWasHit(false);
}

void daEnBlockFlag_c::block_downmove() {
    if (mInitialY <= mPos.y)
        blockWasHit(true);
}

void daEnBlockFlag_c::equaliseEvents() {
    if (mFlagMode != SWAP_EVENTS)
        return;

    bool f1 = checkEvent(mEventID1);
    bool f2 = checkEvent(mEventID2);

    if (!f1 && !f2) {
        dSwitchFlagMng_c::m_instance->set(mEventID2, 0, true, false, false, false);
    }

    if (f1 && f2) {
        dSwitchFlagMng_c::m_instance->set(mEventID1, 0, false, false, false, false);
    }
}


void daEnBlockFlag_c::blockWasHit(bool isDown) {
    mPos.y = mInitialY;

    if (mFlagMode == TOGGLE_EVENT) {
        if (checkEvent(mEventID1))
            dSwitchFlagMng_c::m_instance->set(mEventID1, 0, false, false, false, false);
        else
            dSwitchFlagMng_c::m_instance->set(mEventID1, 0, true, false, false, false);

    } else if (mFlagMode == SWAP_EVENTS) {
        if (checkEvent(mEventID2)) {
            dSwitchFlagMng_c::m_instance->set(mEventID2, 0, false, false, false, false);
            dSwitchFlagMng_c::m_instance->set(mEventID1, 0, true, false, false, false);
        } else {
            dSwitchFlagMng_c::m_instance->set(mEventID2, 0, true, false, false, false);
            dSwitchFlagMng_c::m_instance->set(mEventID1, 0, false, false, false, false);
        }
    }

    mBg.set(this, &l_eventblock_info, 3, mLayer, nullptr);
    mBg.entry();
    
    changeState(StateID_Wait);
}

void daEnBlockFlag_c::initializeState_Wait() {}

void daEnBlockFlag_c::finalizeState_Wait() {}

void daEnBlockFlag_c::executeState_Wait() {
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