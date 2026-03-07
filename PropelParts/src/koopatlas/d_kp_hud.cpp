#include <kamek.h>
#include <propelparts/game_config.h>

#if defined(KOOPATLAS_DEV_ENABLED) || defined(NEWER_MAP_HUD)
#include <constants/message_list.h>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_game_key.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_save_mng.hpp>
#include <propelparts/bases/koopatlas/d_s_koopatlas.hpp>
#include <propelparts/bases/koopatlas/d_kp_hud.hpp>
#include <propelparts/constants/message_list.h>
#include <propelparts/level_info_utils.hpp>

dKpHud_c *dKpHud_c::m_instance = nullptr;

dKpHud_c *dKpHud_c_classInit() {
    dKpHud_c *c = new dKpHud_c;
    dKpHud_c::m_instance = c;
    return c;
}

// Replace WM_TEST actor
kmWritePointer(0x8098FEA0, &dKpHud_c_classInit);

dKpHud_c::dKpHud_c() {
    mLayoutLoaded = false;
    mFooterVisible = false;
    mControllerType = -1;
}

int dKpHud_c::create() {
    static const char *AnmNameTbl[] = {
        "mapHUD_00_showMain.brlan",
        "mapHUD_00_showHeader.brlan",
        "mapHUD_00_hideAll.brlan",
        "mapHUD_00_unhideAll.brlan"
    };

    static const int ANIME_INDEX_TBL[] = {0, 1, 0, 2, 3};
    static const char *GROUP_NAME_DT[] = {
        "G_Lives",
        "G_Header",
        "G_Footer",
        "G_Hideables",
        "G_Hideables"
    };

    static const char *N_PANE_NAME_TBL[] = {
        "N_IconPos1P_00", "N_IconPos2P_00",
        "N_IconPos3P_00", "N_IconPos4P_00"
    };

    static const char *P_PANE_NAME_TBL[] = {
        "Header_Centre", "Header_Right", "Footer",
        "NormalExitFlag", "SecretExitFlag",
        "StarCoinOff0", "StarCoinOff1", "StarCoinOff2",
        "StarCoinOn0", "StarCoinOn1", "StarCoinOn2",
        "P_marioFace_00", "P_luigiFace_00",
        "P_BkinoFace_00", "P_YkinoFace_00",
        "Star0", "Star1", "Star2"
    };

    static const char *T_PANE_NAME_TBL[] = {
        "LevelName", "LevelNameS",
        "LevelNumber", "LevelNumberS",
        "WorldName", "WorldNameS",
        "StarCoinCounter",
        "T_lifeNumber_00", "T_lifeNumber_01",
        "T_lifeNumber_02", "T_lifeNumber_03",
        "ItemsButtonInfo"
    };

    static const char *T_PANE_FIXED_NAME_TBL[] = {
        "MenuButtonInfo",
        "ItemsButtonInfo"
    };

    static const int MESSAGE_DATA_TBL[] = {
        MSG_CS_MENU,
        MSG_CS_ITEMS
    };

    bool res = mLayout.ReadResource("mapHUD.arc", false);
    if (!res) {
        return NOT_READY;
    }

    mLayout.build("mapHUD_00.brlyt", nullptr);
    mLayout.AnimeResRegister(AnmNameTbl, ARRAY_SIZE(AnmNameTbl));
    mLayout.GroupRegister(GROUP_NAME_DT, ANIME_INDEX_TBL, ARRAY_SIZE(GROUP_NAME_DT));
    mLayout.NPaneRegister(N_PANE_NAME_TBL, mpNullPanes, ARRAY_SIZE(T_PANE_NAME_TBL));
    mLayout.PPaneRegister(P_PANE_NAME_TBL, mpPicturePanes, ARRAY_SIZE(P_PANE_NAME_TBL));
    mLayout.TPaneRegister(T_PANE_NAME_TBL, mpTextBoxes, ARRAY_SIZE(T_PANE_NAME_TBL));
    mLayout.TPaneNameRegister(T_PANE_FIXED_NAME_TBL, MESSAGE_DATA_TBL, BMG_CATEGORY_COURSE_SELECT_GUIDE, ARRAY_SIZE(T_PANE_FIXED_NAME_TBL));

    mLayout.mDrawOrder = 0;

    // Adjust layout position and clip for 4:3
    mLayout.getRootPane()->SetSRTElement(0, -112.0f);
    if (!dGameCom::GetAspectRatio()) {
        mLayout.getRootPane()->SetSRTElement(6, 19.0f/26.0f);
        mLayout.getRootPane()->SetSRTElement(7, 352.0f/456.0f);
        mLayout.findPaneByName("N_zanki_00")->SetSRTElement(0, -26.0f);
        mLayout.mScissorMask.mEnabled = true;
        mLayout.mScissorMask.mPos.x = 0;
        mLayout.mScissorMask.mPos.y = 52;
        mLayout.mScissorMask.mSize.x = 640;
        mLayout.mScissorMask.mSize.y = 352;
    }

    mLayout.AllAnimeEndSetup();
    mLayout.AnimeStartSetup(ANIM_SHOW_LIVES, false);

    mLayout.ReverseAnimeStartSetup(ANIM_SHOW_FOOTER, false);
    mLayout.ReverseAnimeStartSetup(ANIM_SHOW_HEADER, false);
    mLayout.ReverseAnimeStartSetup(ANIM_HIDE_ALL, false);

    mHeaderCol.initTexMap(mpPicturePanes[Header_Right]->GetMaterial()->GetTexMapAry());
    mHeaderCol.copyTexMap(mpPicturePanes[Header_Centre]->GetMaterial()->GetTexMapAry());
    mFooterCol.initTexMap(mpPicturePanes[Footer]->GetMaterial()->GetTexMapAry());

    mLayoutLoaded = true;
    mDispHeader = false;
    mDispFooter = false;

    setFooterInfo();
    restDisp();
    return SUCCEEDED;
}

int dKpHud_c::doDelete() {
    dKpHud_c::m_instance = nullptr;
    if (!mLayoutLoaded) {
        return SUCCEEDED;
    }

    return mLayout.doDelete();
}

int dKpHud_c::execute() {
    if (!mLayoutLoaded) {
        return SUCCEEDED;
    }

    if (mDispHeader && (!(mLayout.isAnime(ANIM_SHOW_HEADER)))) {
        mDispHeader = false;
        setHeaderInfo();
        playShowAnim(ANIM_SHOW_HEADER);
    }

    if (mDispFooter && (!(mLayout.isAnime(ANIM_SHOW_FOOTER)))) {
        mDispFooter = false;
        mFooterVisible = true;
        setFooterInfo();
        playShowAnim(ANIM_SHOW_FOOTER);
    }

    restDisp();
    controllerConnectCheck();
    if (mDrawStarEffect && mFooterVisible) {
        drawStarEffects();
    }

    int scCount = dGameCom::getUnspentStarCoinCount();
    dGameCom::LayoutDispNumberDigit(scCount, mpTextBoxes[StarCoinCounter], false);

    mLayout.AnimePlay();
    mLayout.calc();
    return SUCCEEDED;
}

int dKpHud_c::draw() {
    if (mLayoutLoaded) {
        mLayout.entry();
    }
    return SUCCEEDED;
}

void dKpHud_c::doInitialDisp() {
    if (mInitalDispComplete) {
        return;
    }

    mInitalDispComplete = true;

    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    dWorldInfo_c::world_s *world = dWorldInfo_c::m_instance.getWorld(save->mWorldInfoIdx);
    mDispFooter = /*(save->newerWorldName[0] != 0) &&*/ (world->mHudHue != 2000);

    if (!dScKoopatlas_c::m_instance->mPathManager.mIsMoving) {
        enterNode();
    }
}

#ifdef KOOPATLAS_DEV_ENABLED
void dKpHud_c::enterNode(dKpNode_s *node) {
    if (node == nullptr) {
        node = dScKoopatlas_c::m_instance->mPathManager.mpCurrentNode;
    }

    if (node->mNodeType == dKpNode_s::LEVEL && mInitalDispComplete) {
        mDispHeader = true;
        mpHeaderNode = node;
    }
}
#elif defined(NEWER_MAP_HUD)
void dKpHud_c::enterNode(int world, int course) {
    mDispHeader = true;
    mWorldNo = world;
    mCourseNo = course;
}
#endif

void dKpHud_c::exitNode() {
    if (mLayout.GetAnimGroup()[ANIM_SHOW_HEADER].mFrameCtrl.mCurrFrame > 0.1f) {
        // Not hidden
        if ((mLayout.isAnime(ANIM_SHOW_HEADER)
                && !(mLayout.GetAnimGroup()[ANIM_SHOW_HEADER].mFrameCtrl.mFlags & m2d::FrameCtrl_c::REVERSE))
                || (!mLayout.isAnime(ANIM_SHOW_HEADER))) {
            // Currently being shown, OR fully shown already
            playHideAnim(ANIM_SHOW_HEADER);
        }
    }
}

void dKpHud_c::offHudDisp() {
    if (!mLayout.isAnime(ANIM_HIDE_ALL)) {
        mLayout.AnimeStartSetup(ANIM_HIDE_ALL, false);
    }
    mLayout.GetAnimGroup()[ANIM_HIDE_ALL].mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
}

void dKpHud_c::onHudDisp() {
    if (!mLayout.isAnime(ANIM_HIDE_ALL)) {
        mLayout.AnimeStartSetup(ANIM_HIDE_ALL, true);
    }
    mLayout.GetAnimGroup()[ANIM_HIDE_ALL].mFrameCtrl.mFlags = (m2d::FrameCtrl_c::NO_LOOP | m2d::FrameCtrl_c::REVERSE);
}

void dKpHud_c::offFooterDisp() {
    if (mFooterVisible) {
        playHideAnim(ANIM_SHOW_FOOTER);
        mDrawStarEffect = false;
    }
}

void dKpHud_c::onFooterDisp() {
    if (!mInitalDispComplete) {
        return;
    }

    mDispFooter = true;
    if (mFooterVisible) {
        playHideAnim(ANIM_SHOW_FOOTER);
    }
}

void dKpHud_c::restDisp() {
    static const int LogicalPlayerIDs[] = {0,1,3,2};

    mpPicturePanes[P_marioFace_00]->SetVisible(false);
    mpPicturePanes[P_luigiFace_00]->SetVisible(false);
    mpPicturePanes[P_BkinoFace_00]->SetVisible(false);
    mpPicturePanes[P_YkinoFace_00]->SetVisible(false);

    int plyNum = 0;
    for (int i = 0; i < 4; i++) {
        int playerID = LogicalPlayerIDs[i];
        int slotID = SearchForIndexOfPlayerID(playerID);
        int lives = daPyMng_c::mRest[slotID];
        int length = 2;

        dGameCom::LayoutDispNumber(lives, length, mpTextBoxes[T_lifeNumber_00+slotID], true);

        // Display active players
        if (dGameCom::PlayerEnterCheck(slotID)) {
            plyNum++;

            nw4r::lyt::Pane *facePane = mpPicturePanes[P_marioFace_00+playerID];
            facePane->SetTranslate(mpNullPanes[plyNum-1]->GetTranslate());
            facePane->SetVisible(true);
        }
    }

    // Hide/show appropriate frames
    for (int i = 0; i < 4; i++) {
        mpNullPanes[i]->SetVisible(false);
    }
    mpNullPanes[plyNum-1]->SetVisible(true);
}

void dKpHud_c::controllerConnectCheck() {
    int type = dGameKey_c::m_instance->mRemocon[0]->mAttachedExtension;

    if (type != mControllerType) {
        mControllerType = type;
        dInfo_c::m_instance->mExtensionAttached = (type == 0) ? 0 : 1;

        MsgRes_c *msgRes = dMessage_c::getMesRes();
        mpTextBoxes[ItemsButtonInfo]->setMessage(msgRes, BMG_CATEGORY_COURSE_SELECT_GUIDE, MSG_CS_ITEMS, 0);
    }
}

void dKpHud_c::setFooterInfo() {
    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    dWorldInfo_c::world_s *world = dWorldInfo_c::m_instance.getWorld(save->mWorldInfoIdx);
    MsgRes_c *msgRes = dMessage_c::getMesRes();

    const wchar_t *worldName = getKoopatlasWorldName(world->mWorldNameMsgID);
    mpTextBoxes[WorldName]->SetString(worldName, 0);
    mpTextBoxes[WorldNameS]->SetString(worldName, 0);

    mpTextBoxes[WorldName]->SetTextColor(0, world->mHudTextColors[0]);
    mpTextBoxes[WorldName]->SetTextColor(1, world->mHudTextColors[1]);

    mFooterCol.setColor(world->mHudHue%1000, world->mHudSat, world->mHudLight);

    // Figure out if stars are needed
    // Star 0: World is complete
    // Star 1: All exits complete
    // Star 2: All star coins obtained

    bool starVisibility[3];
    starVisibility[0] = false;

    dLevelInfo_c *linfo = &dLevelInfo_c::m_instance;
    dLevelInfo_c::entry_s *lastLevel = linfo->getEntryFromDispID(world->mLevelInfoID, world->mLastLevelID);
    if (lastLevel) {
        starVisibility[0] = (save->getCourseDataFlag(lastLevel->mWorldSlot,lastLevel->mLevelSlot) & dMj2dGame_c::GOAL_NORMAL);
    }

    starVisibility[1] = true;
    starVisibility[2] = true;

    dLevelInfo_c::section_s *sect = linfo->getSection(world->mLevelInfoID);

    for (int i = 0; i < sect->mLevelCount; i++) {
        dLevelInfo_c::entry_s *entry = &sect->mLevels[i];

        if (entry->mFlag & dLevelInfo_c::FLAG_VALID_LEVEL) {
            u32 conds = save->getCourseDataFlag(entry->mWorldSlot, entry->mLevelSlot);

            // Check if we have a goal type, and if its uncleared
            bool noGoalNormal = ((entry->mFlag & dLevelInfo_c::FLAG_GOAL_NORMAL) && !(conds & dMj2dGame_c::GOAL_NORMAL));
            bool noGoalSecret = ((entry->mFlag & dLevelInfo_c::FLAG_GOAL_SECRET) && !(conds & dMj2dGame_c::GOAL_SECRET));

            if (noGoalNormal || noGoalSecret) {
                starVisibility[1] = false;
            }

            if ((conds & dMj2dGame_c::COIN_MASK) != dMj2dGame_c::COIN_MASK) {
                starVisibility[2] = false;
            }
        }
    }

#ifdef KP_HIDE_STARS_W15
    // Newer: Disable stars for the unused Cutland map
    if (world->mLevelInfoID == 15) {
        starVisibility[0] = false;
        starVisibility[1] = false;
        starVisibility[2] = false;
    }
#endif

    // Figure out how many stars are visible
    nw4r::math::VEC3 trans = mpPicturePanes[Star0]->GetTranslate();
    float startX = trans.x;
    for (int i = 0; i < 3; i++) {
        mpPicturePanes[Star0+i]->SetVisible(starVisibility[i]);
        mpPicturePanes[Star0+i]->SetSRTElement(0, startX);
        if (starVisibility[i]) {
            nw4r::lyt::Size size = mpPicturePanes[Star0+i]->GetSize();
            startX += size.width + 4.0f;
        }
    }

    // Position the world name after the stars
    mpTextBoxes[WorldName]->SetSRTElement(0,  startX + 4.0f);
    mpTextBoxes[WorldNameS]->SetSRTElement(0, startX + 6.0f);

    // Draw effects if all stars are visible
    if (starVisibility[0] && starVisibility[1] && starVisibility[2]) {
        mDrawStarEffect = true;
    }
}

void dKpHud_c::setHeaderInfo() {
    dLevelInfo_c *levelInfo = &dLevelInfo_c::m_instance;
#ifdef KOOPATLAS_DEV_ENABLED
    dLevelInfo_c::entry_s *infEntry = levelInfo->getEntryFromSlotID(mpHeaderNode->mLevelNum[0]-1, mpHeaderNode->mLevelNum[1]-1);
#else
    dLevelInfo_c::entry_s *infEntry = levelInfo->getEntryFromSlotID(mWorldNo, mCourseNo);
#endif

    u32 worldIdx = dSaveMng_c::m_instance->getSaveGame(-1)->mWorldInfoIdx;
    dWorldInfo_c::world_s *world = dWorldInfo_c::m_instance.getWorld(worldIdx);
    mHeaderCol.setColor(world->mHudHue%1000, world->mHudSat, world->mHudLight);

    if (infEntry == nullptr) {
        const wchar_t *dummyName = getLevelName(0, 0);
        mpTextBoxes[LevelName]->SetString(dummyName, 0);
        mpTextBoxes[LevelNameS]->SetString(dummyName, 0);

        mpTextBoxes[LevelNumber]->SetString(L"?-?", 0);
        mpTextBoxes[LevelNumberS]->SetString(L"?-?", 0);

        // Hide exit flags and Star Coins
        for (int i = 0; i < 5; i++) {
            mpPicturePanes[NormalExitFlag+i]->SetVisible(false);
        }
        return;
    }

    MsgRes_c *msgRes = dMessage_c::getMesRes();

    // Set the level name
    const wchar_t *levelName = getLevelName(infEntry->mDisplayWorld, infEntry->mDisplayLevel);
    mpTextBoxes[LevelName]->SetString(levelName, 0);
    mpTextBoxes[LevelNameS]->SetString(levelName, 0);

    // Set the level number
    ulong number = getLevelNumberIdx(infEntry->mDisplayWorld, infEntry->mDisplayLevel, infEntry->mWorldSlot, infEntry->mLevelSlot, false);
    mpTextBoxes[LevelNumber]->setMessage(msgRes, BMG_CATEGORY_LEVEL_NAMES, number, 0);
    mpTextBoxes[LevelNumberS]->setMessage(msgRes, BMG_CATEGORY_LEVEL_NAMES, number, 0);

    // Get the level num string width
    nw4r::ut::WideTextWriter lvlNumTextWriter = setupTextWriter(mpTextBoxes[LevelNumber]);
    const wchar_t *numStr = mpTextBoxes[LevelNumber]->GetStringBuffer();
    float bottomWidth = lvlNumTextWriter.CalcStringWidth(numStr, wcslen(numStr));
    nw4r::math::VEC3 trans = mpTextBoxes[LevelNumber]->GetTranslate();
    bottomWidth += trans.x + 12.0f;

    // Set flag and coin data
    clearFlagSet(infEntry, &bottomWidth);
    collectionCoinSet(infEntry, &bottomWidth);

    // Get the width of the level name
    nw4r::ut::WideTextWriter lvlNameTextWriter = setupTextWriter(mpTextBoxes[LevelName]);
    float width = lvlNameTextWriter.CalcStringWidth(levelName, wcslen(levelName));
    nw4r::math::VEC3 levelNameTrans = mpTextBoxes[LevelName]->GetTranslate();
    float topWidth = width + levelNameTrans.x - 20.0f;

    // If the level name is shorter than the bottom row, use bottom width instead
    if (topWidth < bottomWidth) {
        topWidth = bottomWidth;
    }

    // Now set the size of the header backing
    mpPicturePanes[Header_Centre]->SetSRTElement(8, topWidth);
    mpPicturePanes[Header_Right]->SetSRTElement(0, topWidth);
}

void dKpHud_c::playShowAnim(int id) {
    if (!mLayoutLoaded) {
        return;
    }

    mLayout.AnimeStartSetup(id, false);
}

void dKpHud_c::playHideAnim(int id) {
    if (!mLayoutLoaded) {
        return;
    }

    if (!mLayout.isAnime(id)) {
        mLayout.AnimeStartSetup(id, true);
    }

    // Smoothly reverse the animation if its already active
    mLayout.GetAnimGroup()[id].mFrameCtrl.mFlags = (m2d::FrameCtrl_c::NO_LOOP | m2d::FrameCtrl_c::REVERSE);
    if (id == ANIM_SHOW_FOOTER) {
        mFooterVisible = false;
    }
}

nw4r::ut::WideTextWriter dKpHud_c::setupTextWriter(LytTextBox_c *textbox) {
    nw4r::ut::WideTextWriter tWriter;
    tWriter.SetFont(*textbox->GetFont());

    nw4r::lyt::Size fontSize = textbox->GetFontSize();
    tWriter.SetFontSize(fontSize.width, fontSize.height);
    tWriter.SetLineSpace(textbox->GetLineSpace());
    tWriter.SetCharSpace(textbox->GetCharSpace());
    if (textbox->GetTagProcessor() != nullptr) {
        tWriter.SetTagProcessor(textbox->GetTagProcessor());
    }

    return tWriter;
}

void dKpHud_c::clearFlagSet(dLevelInfo_c::entry_s *entry, float *currPos) {
    u32 conds = dSaveMng_c::m_instance->getSaveGame(-1)->getCourseDataFlag(entry->mWorldSlot, entry->mLevelSlot);

    FLAG_STATE_e flagStates[2] = {HIDDEN, HIDDEN}; // First is normal, next is secret

    if ((conds & dMj2dGame_c::GOAL_MASK) && (entry->mFlag & dLevelInfo_c::FLAG_GOAL_MASK) == 0x30) {
        // If this level has two exits but only one of them is collected, show the faded flags
        flagStates[0] = FADED;
        flagStates[1] = FADED;
    }

    // Now check for individual completions
    if ((conds & dMj2dGame_c::GOAL_NORMAL) && (entry->mFlag & dLevelInfo_c::FLAG_GOAL_NORMAL)) {
        flagStates[0] = VISIBLE;
    }
    if ((conds & dMj2dGame_c::GOAL_SECRET) && (entry->mFlag & dLevelInfo_c::FLAG_GOAL_SECRET)) {
        flagStates[1] = VISIBLE;
    }

    // Now set their visibility
    for (int i = 0; i < 2; i++) {
        mpPicturePanes[NormalExitFlag+i]->SetSRTElement(0, *currPos);
        mpPicturePanes[NormalExitFlag+i]->SetAlpha((flagStates[i] == FADED) ? 80 : 255);
        mpPicturePanes[NormalExitFlag+i]->SetVisible(flagStates[i] > HIDDEN);

        if (flagStates[i] > HIDDEN) {
            nw4r::lyt::Size size = mpPicturePanes[NormalExitFlag+i]->GetSize();
            *currPos += size.width;
        }
    }
}

void dKpHud_c::collectionCoinSet(dLevelInfo_c::entry_s *entry, float *currPos) {
    u32 conds = dSaveMng_c::m_instance->getSaveGame(-1)->getCourseDataFlag(entry->mWorldSlot, entry->mLevelSlot);

    // Set visible star coins
    bool haveSC = (entry->mFlag & dLevelInfo_c::FLAG_VALID_LEVEL);

    for (int i = 0; i < 3; i++) {
        bool flag = (conds & (dMj2dGame_c::COIN1_COLLECTED << i));
        mpPicturePanes[StarCoinOn0+i]->SetVisible(flag);
        mpPicturePanes[StarCoinOff0+i]->SetVisible(haveSC);

        // Hide outline if we have the coin (its visible during anims)
        mpPicturePanes[StarCoinOff0+i]->SetAlpha((flag) ? 0 : 0xFF);

        if (haveSC) {
            mpPicturePanes[StarCoinOff0+i]->SetSRTElement(0, *currPos);
            nw4r::lyt::Size size = mpPicturePanes[StarCoinOff0+i]->GetSize();
            *currPos += size.width + 4.0f;
        }
    }
}

void dKpHud_c::drawStarEffects() {
    for (int i = 0; i < 3; i++) {
        nw4r::math::MTX34 glbMtx = mpPicturePanes[Star0+i]->GetGlobalMtx();
        mVec3_c effPos(glbMtx[0][3], glbMtx[1][3], 0.0f);
        dGameCom::getEffPosToLyt(effPos, false);

        mStarEffects[i].createEffect("Wm_2d_titlestar01", 0, &effPos, nullptr, nullptr);
        mStarEffects2[i].createEffect("Wm_2d_titlestar02", 0, &effPos, nullptr, nullptr);
    }
}
#endif