#include <kamek.h>
#include <propelparts/bases/d_a_en_block_switch.hpp>
#include <propelparts/bases/d_custom_profile.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_objblock_mng.hpp>
#include <game/bases/d_s_stage.hpp>

CUSTOM_ACTOR_PROFILE(EN_BLOCK_SWITCH, daEnBlockSwitch_c, fProfile::EN_BLOCK, fProfile::DRAW_ORDER::EN_BLOCK, 0);

STATE_DEFINE(daEnBlockSwitch_c, Wait);
STATE_DEFINE(daEnBlockSwitch_c, HitWait);
STATE_DEFINE(daEnBlockSwitch_c, Dotted);

const dActorData_c c_BLOCK_SWITCH_actor_data = {fProfile::EN_BLOCK_SWITCH, 8, -8, 8, -8, 16, 16, 0, 0, 0, 0, ACTOR_CREATE_MAPOBJ};
dCustomProfile_c l_BLOCK_SWITCH_profile(&g_profile_EN_BLOCK_SWITCH, "EN_BLOCK_SWITCH", CourseActor::EN_BLOCK_SWITCH, &c_BLOCK_SWITCH_actor_data);

sBgSetInfo l_switchblock_bgc_info = {
    mVec2_c(-8, 8),
    mVec2_c(8, -8),
    &daEnBlockMain_c::callBackF,
    &daEnBlockMain_c::callBackH,
    &daEnBlockMain_c::callBackW
};

int daEnBlockSwitch_c::create() {
    Block_CreateClearSet(mPos.y);

    // Set collider
    mBg.set(this, &l_switchblock_bgc_info, 3, mLayer, nullptr);
    mBg.mFlags = 0x260;

    mBg.mBelowCheckFunc = &daEnBlockMain_c::checkRevFoot;
    mBg.mAboveCheckFunc = &daEnBlockMain_c::checkRevHead;
    mBg.mAdjCheckFunc = &daEnBlockMain_c::checkRevWall;

    mBg.mBelowCallback = &daEnBlockMain_c::callBackF;
    mBg.mAboveCallback = &daEnBlockMain_c::callBackH;
    mBg.mAdjCallback = &daEnBlockMain_c::callBackW;

    mPalaceType = (mParam & 0xF) + 1;
    mIsDotted = !(dInfo_c::m_instance->mWmSwitch & (1 << mPalaceType));

    mSwitchType = (dPSwManager_c::SwType_e)(mPalaceType + 1);

    // Setup tile renderer
    dPanelObjMgr_c *list = dBg_c::m_bg_p->getPanelObjMgr(0);
    list->addPanelObjList(&mTile);

    mTile.mPos.x = mPos.x - 8;
    mTile.mPos.y = -(8 + mPos.y);
    mTile.mTileNumber = 0xB5 + mPalaceType - (mIsDotted * 0x10);

    mInitialPos.set(mPos.x, mPos.y, 500.0f);

    int blockCheck = dObjBlockMng_c::m_instance->create_objblock_check(&mInitialPos, dScStage_c::m_instance->mCurrFile);
    switch (blockCheck) {
        case 1:
            changeState(StateID_HitWait);
            break;
        case 2:
            mGiveCoin = true;
        default:
            if (mIsDotted) {
                changeState(StateID_Dotted);
            } else {
                changeState(StateID_Wait);
            }
    }
    
    return SUCCEEDED;
}


int daEnBlockSwitch_c::doDelete() {
    // Remove tile renderer and collider
    dPanelObjMgr_c *list = dBg_c::m_bg_p->getPanelObjMgr(0);
    list->removePanelObjList(&mTile);

    mBg.release();
    return SUCCEEDED;
}


int daEnBlockSwitch_c::execute() {
    mStateMgr.executeState();
    mBg.calc();
    Block_ExecuteClearSet();

    // Update tile collider
    mTile.setPos(mPos.x-8, -(8+mPos.y), mPos.z);
    mTile.setScaleFoot(mScale.x);

    if (mStateMgr.getStateID()->isEqual(StateID_Wait)
        || mStateMgr.getStateID()->isEqual(StateID_Dotted)
        || mStateMgr.getStateID()->isEqual(StateID_HitWait)) {
        ActorScrOutCheck(SKIP_NONE);
    }

    return true;
}

void daEnBlockSwitch_c::initialize_upmove() {
    // Create propeller on block hit
    if (mPalaceType == 3 || mGiveCoin) {
        createItem();
    }
}

void daEnBlockSwitch_c::initialize_downmove() {
    // Same as upmove
    if (mPalaceType == 3 || mGiveCoin) {
        createItem();
    }
}

void daEnBlockSwitch_c::block_upmove() {
    if (mInitialY >= mPos.y) {
        blockWasHit(false);
    }
}

void daEnBlockSwitch_c::block_downmove() {
    if (mInitialY <= mPos.y) {
        blockWasHit(true);
    }
}

void daEnBlockSwitch_c::blockWasHit(bool isDown) {
    mPos.y = mInitialY;
    
    if (mPalaceType != 3 && !mGiveCoin) {
        createItem();
    }
    changeState(StateID_HitWait);
}

void daEnBlockSwitch_c::createItem() {
    mVec3_c mItemPos = mVec3_c(mPos.x, mPos.y-8.0f, mPos.z);
    u32 itemID = l_item_values[mPalaceType];
    if (mGiveCoin) {
        itemID = 2;
    }
    dActor_c::construct(fProfile::EN_ITEM, mPlayerID << 16 | (mIsGroundPound * 3) << 18 | itemID & 0b11111, &mItemPos, nullptr, mLayer);
    // Play item spawn sound
    item_sound_set(mPos, itemID, mPlayerID, 0, 0);
}

void daEnBlockSwitch_c::initializeState_Wait() {
    mBg.entry();
    mTile.mTileNumber = 0xB5 + mPalaceType;
}

void daEnBlockSwitch_c::finalizeState_Wait() {}

void daEnBlockSwitch_c::executeState_Wait() {
    // Revert to a dotted block if our switch is deactivated
    if (mIsDotted && !dPSwManager_c::ms_instance->checkSwitch(mSwitchType)) {
        changeState(StateID_Dotted);
    }

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

void daEnBlockSwitch_c::initializeState_HitWait() {
    mTile.mTileNumber = 0x32;
    dObjBlockMng_c::m_instance->set_objblock_check(&mInitialPos, dScStage_c::m_instance->mCurrFile, 0, 0);
}

void daEnBlockSwitch_c::finalizeState_HitWait() {}

void daEnBlockSwitch_c::executeState_HitWait() {}

void daEnBlockSwitch_c::initializeState_Dotted() {
    mBg.release();
    mTile.mTileNumber = 0xA5 + mPalaceType;
}

void daEnBlockSwitch_c::finalizeState_Dotted() {}

void daEnBlockSwitch_c::executeState_Dotted() {
    // Become solid if our switch is activated
    if (dPSwManager_c::ms_instance->checkSwitch(mSwitchType)) {
        changeState(StateID_Wait);
    }
}