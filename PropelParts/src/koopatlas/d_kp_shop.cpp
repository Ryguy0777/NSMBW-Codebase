#include <kamek.h>
#include <propelparts/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <constants/sound_list.h>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_game_key.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/mLib/m_pad.h>
#include <lib/MSL/math.h>
#include <propelparts/bases/koopatlas/d_kp_shop.hpp>
#include <propelparts/constants/koopatlas_constants.h>
#include <propelparts/constants/message_list.h>
#include <propelparts/level_info_utils.hpp>

STATE_DEFINE(daKpShop_c, Hidden);
STATE_DEFINE(daKpShop_c, ShowWait);
STATE_DEFINE(daKpShop_c, ButtonActivateWait);
STATE_DEFINE(daKpShop_c, CoinCountdown);
STATE_DEFINE(daKpShop_c, Wait);
STATE_DEFINE(daKpShop_c, HideWait);

daKpShop_c *daKpShop_c_classInit() {
    return new daKpShop_c;
}

// Replace WM_TOGEZO actor
kmWritePointer(0x80986074, &daKpShop_c_classInit);

daKpShop_c::daKpShop_c() : mStateMgr(*this, StateID_Hidden) {
    mHasLayoutLoaded = false;
}

int daKpShop_c::create() {
    if (!mHasLayoutLoaded) {
        bool res = mLayout.ReadResource("shop.arc", false);
        if (!res) {
            return NOT_READY;
        }

        mLayout.build("shop_00.brlyt", nullptr);

        if (dGameCom::GetAspectRatio()) {
            mLayout.getRootPane()->SetSRTElement(6, 0.735f);
        } else {
            mLayout.mScissorMask.mEnabled = true;
            mLayout.mScissorMask.mPos.x = 0;
            mLayout.mScissorMask.mPos.y = 52;
            mLayout.mScissorMask.mSize.x = 640;
            mLayout.mScissorMask.mSize.y = 352;

            mLayout.getRootPane()->SetSRTElement(6, 0.731f);
            mLayout.getRootPane()->SetSRTElement(7, 0.7711f);
        }

        static const char *AnmNameTbl[] = {
            "shop_00_Show.brlan",
            "shop_00_Hide.brlan",
            "shop_00_OnButton.brlan",
            "shop_00_OffButton.brlan",
            "shop_00_CountCoin.brlan"
        };

        static const char *GROUP_NAME_DT[] = {
            "BaseGroup",
            "BaseGroup",
            "GBtn00", "GBtn01", "GBtn02", "GBtn03", "GBtn1", "GBtn2",
            "GBtn00", "GBtn01", "GBtn02", "GBtn03", "GBtn1", "GBtn2",
            "GCoinCount"
        };

        static const int ANIME_INDEX_TBL[] = {
            0,
            1,
            2, 2, 2, 2, 2, 2,
            3, 3, 3, 3, 3, 3,
            4
        };

        static const char *N_PANE_NAME_TBL[] = {
            "Btn00", "Btn01", "Btn02", "Btn03", "Btn1", "Btn2"
        };

        static const char *P_PANE_NAME_TBL[] = {
            "BtnLeft00", "BtnLeft01", "BtnLeft02", "BtnLeft03", "BtnLeft1", "BtnLeft2",
            "BtnMid00", "BtnMid01", "BtnMid02", "BtnMid03", "BtnMid1", "BtnMid2",
            "BtnRight00", "BtnRight01", "BtnRight02", "BtnRight03", "BtnRight1", "BtnRight2"
        };

        static const char *T_PANE_NAME_TBL[] = {
            "Title", "TitleShadow",
            "CoinCount", "CoinCountShadow"
        };

        static const char *T_PANE_FIXED_NAME_TBL[] = {
            "BackText",
            "BuyText"
        };

        static const int MESSAGE_DATA_TBL[] = {
            0x90,
            0x91
        };

        mLayout.AnimeResRegister(AnmNameTbl, ARRAY_SIZE(AnmNameTbl));
        mLayout.GroupRegister(GROUP_NAME_DT, ANIME_INDEX_TBL, ARRAY_SIZE(GROUP_NAME_DT));
        mLayout.NPaneRegister(N_PANE_NAME_TBL, mpNullPanes, ARRAY_SIZE(N_PANE_NAME_TBL));
        mLayout.PPaneRegister(P_PANE_NAME_TBL, mpPicturePanes, ARRAY_SIZE(P_PANE_NAME_TBL));
        mLayout.TPaneRegister(T_PANE_NAME_TBL, mpTextBoxes, ARRAY_SIZE(T_PANE_NAME_TBL));
        mLayout.TPaneNameRegister(T_PANE_FIXED_NAME_TBL, MESSAGE_DATA_TBL, BMG_CATEGORY_KOOPATLAS, ARRAY_SIZE(T_PANE_FIXED_NAME_TBL));
        mLayout.AllAnimeEndSetup();

        mLayout.mDrawOrder = 2;

        // Setup button colors
        mBtnLeftCol.initTexMap(mpPicturePanes[BtnLeft00]->GetMaterial()->GetTexMapAry());
        mBtnMidCol.initTexMap(mpPicturePanes[BtnMid00]->GetMaterial()->GetTexMapAry());
        mBtnRightCol.initTexMap(mpPicturePanes[BtnRight00]->GetMaterial()->GetTexMapAry());

        for (int i = 1; i < 6; i++) {
            mBtnLeftCol.copyTexMap(mpPicturePanes[BtnLeft00+i]->GetMaterial()->GetTexMapAry());
            mBtnMidCol.copyTexMap(mpPicturePanes[BtnMid00+i]->GetMaterial()->GetTexMapAry());
            mBtnRightCol.copyTexMap(mpPicturePanes[BtnRight00+i]->GetMaterial()->GetTexMapAry());
        }

        mCurrSelection = 0;
        mTopRowSelection = 0;
        mIsVisible = false;
        mHasLayoutLoaded = true;
    }

    return SUCCEEDED;
}

int daKpShop_c::doDelete() {
    deleteModels();
    return mLayout.doDelete();
}

int daKpShop_c::execute() {
    mStateMgr.executeState();

    if (mIsVisible) {
        mpLakitu->execute();

        for (int i = 0; i < SHOP_ITEM_COUNT; i++) {
            mpItems[i].execute();
        }

        mLayout.AnimePlay();
        mLayout.calc();
    }
    return SUCCEEDED;
}

int daKpShop_c::draw() {
    if (mIsVisible) {
        mLayout.entry();
    }
    return SUCCEEDED;
}

void daKpShop_c::draw2D() {
    if (mIsVisible) {
        mpLakitu->mScaleEase = mScaleEase;
        mpLakitu->draw();

        for (int i = 0; i < SHOP_ITEM_COUNT; i++) {
            mpItems[i].mScaleEase = mScaleEase;
            mpItems[i].draw();
        }
    }
}


void daKpShop_c::dispMenu(int shopNum) {
    mShopIdx = shopNum;
    mStateMgr.changeState(StateID_ShowWait);
}

void daKpShop_c::loadInfo() {
    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    dWorldInfo_c::world_s *world = dWorldInfo_c::m_instance.getWorld(mShopIdx);
    
    mBtnLeftCol.setColor(world->mHudHue, world->mHudSat, world->mHudLight);
    mBtnMidCol.setColor(world->mHudHue, world->mHudSat, world->mHudLight);
    mBtnRightCol.setColor(world->mHudHue, world->mHudSat, world->mHudLight);

    // Set the shop title
    dLevelInfo_c::entry_s *entry = dLevelInfo_c::m_instance.getEntryFromSlotID(mShopIdx, COURSE_ID_SHOP-1);
    if (entry != nullptr) {
        const wchar_t *levelName = getLevelName(entry->mDisplayWorld, entry->mDisplayLevel);
        mpTextBoxes[Title]->SetString(levelName, 0);
        mpTextBoxes[TitleShadow]->SetString(levelName, 0);
    }

    UnspentCoinSet();
}

void daKpShop_c::loadModels() {
    mpLakitu = new ShopModel_c;
    mpLakitu->createLakitu(mShopIdx);

    mpLakitu->mPos.x = 240.0f;
    mpLakitu->mPos.y = 220.0f;

    // 4:3 adjustments
    if (!dGameCom::GetAspectRatio()) {
        mpLakitu->mPos.x = (0.731f * (mpLakitu->mPos.x + 416.0f)) - 292.0f;
        mpLakitu->mPos.y *= 0.7711f;
        mpLakitu->mPos.y += 52.0f;
    }

    static const float sc_itemPos[SHOP_ITEM_COUNT][2] = {
        {357.0f, 276.0f},
        {450.0f, 276.0f},
        {543.0f, 276.0f},
        {636.0f, 276.0f},

        {380.0f, 190.0f},
        {462.0f, 190.0f},
        {544.0f, 190.0f},

        {363.0f, 104.0f},
        {413.0f, 104.0f},
        {463.0f, 104.0f},
        {513.0f, 104.0f},
        {563.0f, 104.0f},
    };

    mpItems = new ShopModel_c[SHOP_ITEM_COUNT];
    for (int i = 0; i < SHOP_ITEM_COUNT; i++) {
        float effectiveX = sc_itemPos[i][0];
        float effectiveY = sc_itemPos[i][1];

        if (!dGameCom::GetAspectRatio()) {
            effectiveX = (0.731f * (effectiveX + 416.0f)) - 296.0f;
            effectiveY *= 0.7711f;
            effectiveY += 50.0f;
        }

        u8 *itemSet = dWorldInfo_c::m_instance.getShopSet(mShopIdx);
        mpItems[i].createItem(effectiveX, effectiveY, (ITEM_TYPE_e)itemSet[i]);
    }
}

void daKpShop_c::deleteModels() {
    if (mpLakitu) {
        delete mpLakitu;
    }
    mpLakitu = nullptr;

    if (mpItems) {
        delete[] mpItems;
    }
    mpItems = nullptr;
}

void daKpShop_c::buyItem(int item) {
    static const ItemDef_s itemDefs[6] = {
        // Cost, Start Index, Item Count
        {1, 0, 1},
        {2, 1, 1},
        {2, 2, 1},
        {3, 3, 1},
        {5, 4, 3},
        {8, 7, 5}
    };

    int cost = itemDefs[item].mCost;
    int cash = dGameCom::getUnspentStarCoinCount();

    // Not enough funds, deny a purchase
    if (cost > cash) {
        OSReport("Started playing Not Enough\n");
        mpLakitu->mIsNotEnoughAnime = true;
        mpLakitu->setAnm("notenough", 1.0f, m3d::FORWARD_ONCE);
        SndAudioMgr::sInstance->startSystemSe(SE_SYS_INVALID, 1);
        return;
    }

    SndAudioMgr::sInstance->startSystemSe(SE_SYS_DECIDE, 1);

    mBoughtItemCost = cost;

    // Figure out what items to give
    int appliedItems[TYPE_NUM];
    for (int i = 0; i < TYPE_NUM; i++) {
        appliedItems[i] = 0;
    }

    u8 *itemSet = dWorldInfo_c::m_instance.getShopSet(mShopIdx);
    int invStartIndex = itemDefs[item].mIndex;
    int invCount = itemDefs[item].mItemNum;

    for (int i = 0; i < invCount; i++) {
        appliedItems[(int)itemSet[invStartIndex+i]]++;
    }

    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);

    // Apply items
    for (int i = 0; i < POWERUP_COUNT; i++) {
        int newCount = save->getStockItem(i) + appliedItems[i];
        save->setStockItem(i, newCount);

        dScKoopatlas_c::m_instance->mpStockItem->mItemCounts[i] = save->getStockItem(i);
    }

    // Apply lives to active players
    for (int i = 0; i < 4; i++) {
        if (daPyMng_c::mPlayerEntry[i]) {
            int type = daPyMng_c::mPlayerType[i];
            daPyMng_c::mRest[type] += appliedItems[TYPE_ONE_UP];

            if (daPyMng_c::mRest[type] > 99) {
                daPyMng_c::mRest[type] = 99;
            }
        }
    }

    if (appliedItems[(int)TYPE_ONE_UP] > 0) {
        SndAudioMgr::sInstance->startSystemSe(SE_SYS_100COIN_ONE_UP, 1);
    }

    mStateMgr.changeState(StateID_CoinCountdown);
    dSelectCursor_c::m_instance->Cancel(0);
}

int daKpShop_c::UnspentCoinSet() {
    int scCount = dGameCom::getUnspentStarCoinCount();
    dGameCom::LayoutDispNumberDigit(scCount, mpTextBoxes[CoinCount], false);
    dGameCom::LayoutDispNumberDigit(scCount, mpTextBoxes[CoinCountShadow], false);
    return scCount;
}

void daKpShop_c::SelectCursorSetup() {
#ifdef KP_USE_SHOP_CURSORS
    int paneIdx = Btn00 + mCurrSelection;
    dGameCom::SelectCursorSetup(mpNullPanes[paneIdx], 0, false);
#endif
}


// Hidden
void daKpShop_c::initializeState_Hidden() { }
void daKpShop_c::executeState_Hidden() { }
void daKpShop_c::finalizeState_Hidden() { }


// ShowWait
void daKpShop_c::initializeState_ShowWait() {
    SndAudioMgr::sInstance->startSystemSe(SE_SYS_DIALOGUE_IN, 1);

    mLayout.AllAnimeEndSetup();
    mLayout.AnimeStartSetup(ANIM_SHOW_ALL, false);
    mIsVisible = true;
    mScaleEase = 0.0;

    loadInfo();
    loadModels();
}
void daKpShop_c::executeState_ShowWait() {
    if (!mLayout.isAnime(ANIM_SHOW_ALL)) {
        mCurrSelection = 0;
        mStateMgr.changeState(StateID_ButtonActivateWait);
    }
}
void daKpShop_c::finalizeState_ShowWait() {
    SndAudioMgr::sInstance->startSystemSe(SE_OBJ_CLOUD_BLOCK_TO_JUGEM, 1);
    mDelay = 1;
}


// ButtonActivateWait
void daKpShop_c::initializeState_ButtonActivateWait() { }
void daKpShop_c::executeState_ButtonActivateWait() {
    if (!mLayout.isAllAnime()) {
        if (mDelay < 90) {
            mScaleEase = -((cos(mDelay * M_PI / 20) - 0.9) / mDelay * 10) + 1;
            mDelay++;
            return;
        }

        mLayout.AnimeStartSetup(ANIM_ON_BUTTON, false);
        mStateMgr.changeState(StateID_Wait);
    }
}
void daKpShop_c::finalizeState_ButtonActivateWait() { }


// Wait
void daKpShop_c::initializeState_Wait() { }
void daKpShop_c::executeState_Wait() {
    SelectCursorSetup();

    int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;
    int newSelection = -1;

    if (pressed & WPAD_BUTTON_1) {
        // Hide the thing
        mStateMgr.changeState(StateID_HideWait);

    } else if (pressed & WPAD_BUTTON_RIGHT) {
        // Move up
        if (mCurrSelection == 4) {
            newSelection = mTopRowSelection;
        } else if (mCurrSelection == 5) {
            newSelection = 4;
        }

    } else if (pressed & WPAD_BUTTON_LEFT) {
        // Move down
        if (mCurrSelection <= 3) {
            newSelection = 4;
        } else if (mCurrSelection == 4) {
            newSelection = 5;
        }

    } else if (pressed & WPAD_BUTTON_UP) {
        // Just use the previous choice
        if (mCurrSelection > 0) {
            newSelection = mCurrSelection - 1;
        }

    } else if (pressed & WPAD_BUTTON_DOWN) {
        // Just use the next choice
        if (mCurrSelection < 5) {
            newSelection = mCurrSelection + 1;
        }

    } else if (pressed & WPAD_BUTTON_2) {
        if (!mpLakitu->mIsNotEnoughAnime) {
            buyItem(mCurrSelection);
        }
    }

    if (newSelection > -1) {
        SndAudioMgr::sInstance->startSystemSe(SE_SYS_CURSOR, 1);

        mLayout.AnimeStartSetup(ANIM_OFF_BUTTON+mCurrSelection, false);
        mLayout.AnimeStartSetup(ANIM_ON_BUTTON+newSelection, false);

        mCurrSelection = newSelection;
        if (newSelection <= 3) {
            mTopRowSelection = newSelection;
        }

        SelectCursorSetup();
    }
}
void daKpShop_c::finalizeState_Wait() { }


// HideWait
void daKpShop_c::initializeState_HideWait() {
    SndAudioMgr::sInstance->startSystemSe(SE_SYS_DIALOGUE_OUT_AUTO, 1);
    mLayout.AnimeStartSetup(ANIM_HIDE_ALL, false);
    mLayout.AnimeStartSetup(ANIM_OFF_BUTTON+mCurrSelection, false);

    mDelay = 26;
    SndAudioMgr::sInstance->startSystemSe(SE_OBJ_CS_KINOHOUSE_DISAPP, 1);

    dSelectCursor_c::m_instance->Cancel(0);
}
void daKpShop_c::executeState_HideWait() {
    if (mDelay > 0) {
        mDelay--;

        mScaleEase = -((cos(mDelay * M_PI / 13.5) - 0.9) / mDelay * 10) + 1;
        if (mScaleEase < 0.0f) {
            mScaleEase = 0.0f;
        }
    }

    if (!mLayout.isAnime(ANIM_HIDE_ALL)) {
        mStateMgr.changeState(StateID_Hidden);
    }
}
void daKpShop_c::finalizeState_HideWait() {
    deleteModels();
    mIsVisible = false;
}


// CoinCountdown
void daKpShop_c::initializeState_CoinCountdown() {
    mCoinSpendDelay = 8;
}
void daKpShop_c::executeState_CoinCountdown() {
    if (mCoinSpendDelay != 0) {
        mCoinSpendDelay--;
        return;
    }

    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    save->mSpentStarCoins++;

    // Set the new count
    int scCount = UnspentCoinSet();

    // Setup effect
    mLayout.AnimeStartSetup(ANIM_COUNT_COIN, false);
    nw4r::math::MTX34 glbMtx = mpTextBoxes[CoinCount]->GetGlobalMtx();
    mVec3_c effPos(glbMtx[0][3], glbMtx[1][3], 0.0f);

    nw4r::lyt::Size coinSize = mpTextBoxes[CoinCount]->GetSize();

    if (dGameCom::GetAspectRatio()) {
        float div = 5.0f;
        if (scCount < 100) {
            div = 3.6f;
        }
        if (scCount < 10) {
            div = 2.7f;
        }
        effPos.x -= (coinSize.width / div);
        effPos.y -= (coinSize.height / 2.0f);
    } else {
        float div = 5.8f;
        if (scCount < 100) {
            div = 8.2f;
        }
        if (scCount < 10) {
            div = 14.5f;
        }
        effPos.x += (coinSize.width / div);
        effPos.y -= (coinSize.height / 2.8f);
    }

    mVec3_c effScale(0.7f, 0.7f, 0.7f);
    mEf::createEffect("Wm_2d_moviecoinvanish", 0, &effPos, nullptr, &effScale);

    mBoughtItemCost--;
    if (mBoughtItemCost <= 0) {
        SndAudioMgr::sInstance->startSystemSe(SE_PLY_GET_ITEM_AGAIN, 1);
        mStateMgr.changeState(StateID_Wait);
    } else {
        SndAudioMgr::sInstance->startSystemSe(SE_SYS_STAR_COIN_PAY, 1);
        mCoinSpendDelay = 8; // Do this all over again
    }
}
void daKpShop_c::finalizeState_CoinCountdown() { }



const char *daKpShop_c::ShopModel_c::sc_itemResNames[TYPE_NUM][4] = {
    { "I_kinoko",         "g3d/I_kinoko.brres",       "I_kinoko",           "wait2" },
    { "I_fireflower",     "g3d/I_fireflower.brres",   "I_fireflower",       "wait2" },
    { "I_propeller",      "g3d/I_propeller.brres",    "I_propeller_model",  "wait2" },
    { "I_iceflower",      "g3d/I_iceflower.brres",    "I_iceflower",        "wait2" },
    { "I_penguin",        "g3d/I_penguin.brres",      "I_penguin",          "wait2" },
    { "I_kinoko",  "g3d/I_kinoko.brres",  "I_kinoko",      "wait2" },
    { "I_star",           "g3d/I_star.brres",         "I_star",             "wait2" },
    { "I_hammer",         "g3d/I_fireflower.brres",   "I_fireflower",       "wait2" },
    { "I_kinoko",  "g3d/I_kinoko.brres",  "I_kinoko",      "wait2" },
};

const char *daKpShop_c::ShopModel_c::sc_lakituResNames[] = {
    "g3d/yoshi.brres",
    "g3d/desert.brres",
    "g3d/mountain.brres",
    "g3d/sakura.brres",
    "g3d/santa.brres",
    "g3d/ghost.brres",
    "g3d/space.brres",
    "g3d/koopa.brres",
    "g3d/sewer.brres",
    "g3d/goldwood.brres"
};

void daKpShop_c::ShopModel_c::createItem(float x, float y, ITEM_TYPE_e type) {
    mPos.x = x;
    mPos.y = y;
    int itemID = (int)type;

    // Set the scale based on the aspect ratio and if its a mini mushroom
    if (itemID == TYPE_MINI_SHROOM) {
        mScaleFactor = (dGameCom::GetAspectRatio()) ? 1.625f : 1.15f;
    } else {
        mScaleFactor = (dGameCom::GetAspectRatio()) ? 2.3f : 1.6f;
    }

    mIsLakitu = false;

    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes(sc_itemResNames[itemID][0], sc_itemResNames[itemID][1]);
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl(sc_itemResNames[itemID][2]);
    mModel.create(resMdl, &mAllocator, 0x227, 1, 0);

    dGameCom::SetSoftLight_Item(mModel, 1);

    // Setup idle anim
    nw4r::g3d::ResAnmChr resChr = mResFile.GetResAnmChr(sc_itemResNames[itemID][3]);
    mAnmChr.create(resMdl, resChr, &mAllocator, 0);
    setAnm(sc_itemResNames[itemID][3], 1.0f, m3d::FORWARD_LOOP);

    // Setup other mushrooms
    if ((itemID == TYPE_MINI_SHROOM) || (itemID == TYPE_ONE_UP)) {
        nw4r::g3d::ResAnmTexPat resPat = mResFile.GetResAnmTexPat("I_kinoko_switch");
        mAnmTexPat.create(resMdl, resPat, &mAllocator, 0, 1);
        mAnmTexPat.setAnm(mModel, resPat, 0, m3d::FORWARD_ONCE);
        mAnmTexPat.setRate(0.0f, 0);
        mModel.setAnm(mAnmTexPat);

        // Now set the frame we need, 1 is 1-Up, 2 is mini
        float frame = (itemID == TYPE_ONE_UP) ? 1 : 2;
        mAnmTexPat.setFrame(frame, 0);
    }

    mAllocator.adjustFrmHeap();
}

void daKpShop_c::ShopModel_c::createLakitu(int type) {
    mScaleFactor = (dGameCom::GetAspectRatio()) ? 1.0f : 0.77f;
    mScaleEase = 0.0f;
    mIsLakitu = true;

    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[0], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("lakitu", sc_lakituResNames[type]);
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl("lakitu");
    mModel.create(resMdl, &mAllocator, 0x224, 1, 0);

    dGameCom::SetSoftLight_Enemy(mModel, 1);

    // Setup idle anim
    nw4r::g3d::ResAnmChr resChr = mResFile.GetResAnmChr("idle");
    mAnmChr.create(resMdl, resChr, &mAllocator, 0);
    setAnm("idle", 1.0f, m3d::FORWARD_LOOP);

    mIsNotEnoughAnime = false;

    mAllocator.adjustFrmHeap();
}

void daKpShop_c::ShopModel_c::execute() {
    mModel.play();

    if (mAnmChr.isStop() && mIsLakitu) {
        if (mIsNotEnoughAnime) {
            setAnm("idle", 1.0f, m3d::FORWARD_LOOP);
            mAnmChr.setFrame(0.0f);

            mIsNotEnoughAnime = false;
        }
    }
}

void daKpShop_c::ShopModel_c::draw() {
    mMtx_c matrix;
    PSMTXTrans(matrix, mPos.x, mPos.y, 1000.0f);

    float size = mScaleFactor * mScaleEase;
    mVec3_c scale(size, size, size);
    mModel.setScale(scale);

    mModel.setLocalMtx(&matrix);
    mModel.calc(false);

    // Draw model
    mModel.entry();
}

void daKpShop_c::ShopModel_c::setAnm(const char *name, float rate, m3d::playMode_e playMode) {
    nw4r::g3d::ResAnmChr resChr = mResFile.GetResAnmChr(name);
    if (!resChr.ptr()) {
        OSReport("ShopModel: CHR Anim \"%s\" not found!\n", name);
        return;
    }

    mAnmChr.setAnm(mModel, resChr, playMode);
    mModel.setAnm(mAnmChr, 0.0f);
    mAnmChr.setRate(rate);
}
#endif