#include <kamek.h>
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_wm_hud.hpp>
#include <new/level_info_utils.hpp>
#include <new/constants/message_list.h>
#include <constants/message_list.h>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_game_key.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_save_mng.hpp>

dWMHud_c *dWMHud_c::m_instance = nullptr;

dWMHud_c *dWMHud_c::build() {
    dWMHud_c *c = new dWMHud_c;
    m_instance = c;
    return c;
}

// Replace WM_DANCE_PAKKUN actor
kmWritePointer(0x80982844, &dWMHud_c::build);

dWMHud_c::dWMHud_c() {
    mLayoutLoaded = false;
    mControllerType = -1;
    mFooterVisible = false;
}


int dWMHud_c::create() {
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
        mLayout.mScissorMask.mEnabled = true;
        mLayout.mScissorMask.mPos.x = 0;
        mLayout.mScissorMask.mPos.y = 52;
        mLayout.mScissorMask.mSize.x = 640;
        mLayout.mScissorMask.mSize.y = 352;
    }

    mLayout.AllAnimeEndSetup();
    mLayout.AnimeStartSetup(ANIM_SHOW_LIVES, false);

    mHeaderCol.setTexMap(mpPicturePanes[Header_Right]->GetMaterial()->GetTexMapAry());
    mHeaderCol.applyAlso(mpPicturePanes[Header_Centre]->GetMaterial()->GetTexMapAry());
    mFooterCol.setTexMap(mpPicturePanes[Footer]->GetMaterial()->GetTexMapAry());

    mLayoutLoaded = true;
    mDispHeader = false;
    mDispFooter = false;

    loadFooterInfo();
    setupLives();
    return SUCCEEDED;
}


void dWMHud_c::loadInitially() {
    if (mInitalDispComplete)
        return;

    mInitalDispComplete = true;

    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    dWorldInfo_c::world_s *world = dWorldInfo_c::m_instance.getWorld(save->mWorldInfoIdx);
    mDispFooter = /*(save->newerWorldName[0] != 0) &&*/ (world->mHudHue != 2000);

    //if (!dScKoopatlas_c::instance->mPathManager.isMoving)
    //    enteredNode();
}


int dWMHud_c::doDelete() {
    dWMHud_c::m_instance = nullptr;
    if (!mLayoutLoaded) {
        return SUCCEEDED;
    }

    return mLayout.doDelete();
}

int dWMHud_c::execute() {
    if (!mLayoutLoaded) {
        return SUCCEEDED;
    }

    mDispHeader = true; // Temp, remove me once the map data is complete
    if (mDispHeader && (!(mLayout.isAnime(ANIM_SHOW_HEADER)))) {
        mDispHeader = false;
        loadHeaderInfo();
        playShowAnim(ANIM_SHOW_HEADER);
    }

    if (mDispFooter && (!(mLayout.isAnime(ANIM_SHOW_FOOTER)))) {
        mDispFooter = false;
        mFooterVisible = true;
        loadFooterInfo();
        playShowAnim(ANIM_SHOW_FOOTER);
    }

    setupLives();
    controllerConnectCheck();

    int scCount = dGameCom::getUnspentStarCoinCount();
    dGameCom::LayoutDispNumberDigit(scCount, mpTextBoxes[StarCoinCounter], false);

    mLayout.AnimePlay();
    mLayout.calc();
    return SUCCEEDED;
}


int dWMHud_c::draw() {
    if (mLayoutLoaded) {
        mLayout.entry();
    }
    return SUCCEEDED;
}


void dWMHud_c::hideAll() {
    if (!mLayout.isAnime(ANIM_HIDE_ALL))
        mLayout.AnimeStartSetup(ANIM_HIDE_ALL, false);
    mLayout.GetAnimGroup()[ANIM_HIDE_ALL].mFrameCtrl.mFlags = 1; // NO_LOOP
}
void dWMHud_c::unhideAll() {
    if (!mLayout.isAnime(ANIM_HIDE_ALL))
        mLayout.AnimeStartSetup(ANIM_HIDE_ALL, true);
    mLayout.GetAnimGroup()[ANIM_HIDE_ALL].mFrameCtrl.mFlags = 3; // NO_LOOP | REVERSE
}




void dWMHud_c::playShowAnim(int id) {
    if (!this || !this->mLayoutLoaded) return;

    mLayout.AnimeStartSetup(id, false);
}

void dWMHud_c::playHideAnim(int id) {
    if (!this || !this->mLayoutLoaded) return;

    if (!mLayout.isAnime(id)) {
        mLayout.AnimeStartSetup(id, true);
    }
    mLayout.GetAnimGroup()[id].mFrameCtrl.mFlags = 3; // NO_LOOP | REVERSE
    if (id == ANIM_SHOW_FOOTER)
        mFooterVisible = false;
}


void dWMHud_c::loadHeaderInfo() {
    dLevelInfo_c *levelInfo = &dLevelInfo_c::m_instance;
    //dLevelInfo_c::entry_s *infEntry = levelInfo->getEntryFromSlotID(mpHeaderNode->levelNumber[0]-1, mpHeaderNode->levelNumber[1]-1);
    dLevelInfo_c::entry_s *infEntry = levelInfo->getEntryFromSlotID(3, 21); // Temp
    MsgRes_c *msgRes = dMessage_c::getMesRes();

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

    // Set the level name
    const wchar_t *levelName = getLevelName(infEntry->mDisplayWorld, infEntry->mDisplayLevel);
    mpTextBoxes[LevelName]->SetString(levelName, 0);
    mpTextBoxes[LevelNameS]->SetString(levelName, 0);

    // Set the level number
    ulong number = getLevelNumberIdx(infEntry->mDisplayWorld, infEntry->mDisplayLevel, infEntry->mWorldSlot, infEntry->mLevelSlot, false);
    mpTextBoxes[LevelNumber]->setMessage(msgRes, BMG_CATEGORY_LEVEL_NAMES, number, 0);
    mpTextBoxes[LevelNumberS]->setMessage(msgRes, BMG_CATEGORY_LEVEL_NAMES, number, 0);

    // Get the level number width (for the dynamic HUD size)
    nw4r::ut::WideTextWriter tw2;
    tw2.SetFont(*mpTextBoxes[LevelNumber]->GetFont());
    nw4r::lyt::Size fontSize = mpTextBoxes[LevelNumber]->GetFontSize();
    tw2.SetFontSize(fontSize.width, fontSize.height);
    tw2.SetLineSpace(mpTextBoxes[LevelNumber]->GetLineSpace());
    tw2.SetCharSpace(mpTextBoxes[LevelNumber]->GetCharSpace());
    if (mpTextBoxes[LevelNumber]->GetTagProcessor() != nullptr)
        tw2.SetTagProcessor(mpTextBoxes[LevelNumber]->GetTagProcessor());

    const wchar_t *numStrBuf = mpTextBoxes[LevelNumber]->GetStringBuffer();
    float currentPos = tw2.CalcStringWidth(numStrBuf, wcslen(numStrBuf));
    nw4r::math::VEC3 trans = mpTextBoxes[LevelNumber]->GetTranslate();
    currentPos += trans.x + 12.0f;

    // Setup the clearing flags
    int w = infEntry->mWorldSlot;
    int l = infEntry->mLevelSlot;

    u32 conds = dSaveMng_c::m_instance->getSaveGame(-1)->getCourseDataFlag(w, l);
    // States: 0 = invisible, 1 = visible, 2 = faded
    int normalState = 0, secretState = 0;

    if ((conds & dMj2dGame_c::GOAL_MASK) && (infEntry->mFlag & dLevelInfo_c::FLAG_GOAL_MASK) == 0x30) {
        // If this level has two exits and one of them is already collected,
        // then show the faded flags
        normalState = 2;
        secretState = 2;
    }

    if ((conds & dMj2dGame_c::GOAL_NORMAL) && (infEntry->mFlag & dLevelInfo_c::FLAG_GOAL_NORMAL))
        normalState = 1;
    if ((conds & dMj2dGame_c::GOAL_SECRET) && (infEntry->mFlag & dLevelInfo_c::FLAG_GOAL_SECRET))
        secretState = 1;

    mpPicturePanes[NormalExitFlag]->SetSRTElement(0, currentPos);
    mpPicturePanes[NormalExitFlag]->SetAlpha((normalState == 2) ? 80 : 255);
    mpPicturePanes[NormalExitFlag]->SetVisible(normalState > 0);
    if (normalState > 0) {
        nw4r::lyt::Size size = mpPicturePanes[NormalExitFlag]->GetSize();
        currentPos += size.width;
    }

    mpPicturePanes[SecretExitFlag]->SetSRTElement(0, currentPos);
    mpPicturePanes[SecretExitFlag]->SetAlpha((secretState == 2) ? 80 : 255);
    mpPicturePanes[SecretExitFlag]->SetVisible(secretState > 0);
    if (secretState > 0) {
        nw4r::lyt::Size size = mpPicturePanes[SecretExitFlag]->GetSize();
        currentPos += size.width;
    }

    // Setup the Star Coins
    bool haveSC = (infEntry->mFlag & dLevelInfo_c::FLAG_VALID_LEVEL);

    for (int i = 0; i < 3; i++) {
        bool flag = (conds & (dMj2dGame_c::COIN1_COLLECTED << i));
        mpPicturePanes[StarCoinOn0+i]->SetVisible(flag);
        mpPicturePanes[StarCoinOff0+i]->SetVisible(haveSC);
        // Hide outline if we have the coin (it can be seen during anims)
        mpPicturePanes[StarCoinOff0+i]->SetAlpha((flag) ? 0 : 0xFF);

        if (haveSC) {
            mpPicturePanes[StarCoinOff0+i]->SetSRTElement(0, currentPos);
            nw4r::lyt::Size size = mpPicturePanes[StarCoinOff0+i]->GetSize();
            currentPos += size.width + 4.0f;
        }
    }

    // Get the width of the level name
    nw4r::ut::WideTextWriter tw;
    tw.SetFont(*mpTextBoxes[LevelName]->GetFont());
    nw4r::lyt::Size nameFontSize = mpTextBoxes[LevelName]->GetFontSize();
    tw.SetFontSize(nameFontSize.width, nameFontSize.height);
    tw.SetLineSpace(mpTextBoxes[LevelName]->GetLineSpace());
    tw.SetCharSpace(mpTextBoxes[LevelName]->GetCharSpace());
    if (mpTextBoxes[LevelName]->GetTagProcessor() != nullptr)
        tw.SetTagProcessor(mpTextBoxes[LevelName]->GetTagProcessor());

    float width = tw.CalcStringWidth(levelName, wcslen(levelName));
    nw4r::math::VEC3 levelNameTrans = mpTextBoxes[LevelName]->GetTranslate();

    float totalWidth = width + levelNameTrans.x - 20.0f;
    // If the level name is shorter than the bottom row,
    // use that instead
    if (totalWidth < currentPos)
        totalWidth = currentPos;

    mpPicturePanes[Header_Centre]->SetSRTElement(8, totalWidth);
    mpPicturePanes[Header_Right]->SetSRTElement(0, totalWidth);

    u32 worldIdx = dSaveMng_c::m_instance->getSaveGame(-1)->mWorldInfoIdx;
    dWorldInfo_c::world_s *world = dWorldInfo_c::m_instance.getWorld(worldIdx);
    mHeaderCol.colourise(world->mHudHue%1000, world->mHudSat, world->mHudLight);
}


void dWMHud_c::loadFooterInfo() {
    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    dWorldInfo_c::world_s *world = dWorldInfo_c::m_instance.getWorld(save->mWorldInfoIdx);
    MsgRes_c *msgRes = dMessage_c::getMesRes();

    const wchar_t *worldName = getKoopatlasWorldName(world->mWorldNameMsgID);
    mpTextBoxes[WorldName]->SetString(worldName, 0);
    mpTextBoxes[WorldNameS]->SetString(worldName, 0);

    mpTextBoxes[WorldName]->SetTextColor(0, world->mHudTextColors[0]);
    mpTextBoxes[WorldName]->SetTextColor(1, world->mHudTextColors[1]);

    mFooterCol.colourise(world->mHudHue%1000, world->mHudSat, world->mHudLight);

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

            if (((entry->mFlag & dLevelInfo_c::FLAG_GOAL_NORMAL) && !(conds & dMj2dGame_c::GOAL_NORMAL)) ||
                    ((entry->mFlag & dLevelInfo_c::dLevelInfo_c::FLAG_GOAL_SECRET) && !(conds & dMj2dGame_c::GOAL_SECRET)))
                        starVisibility[1] = false;

            if ((conds & dMj2dGame_c::COIN_MASK) != dMj2dGame_c::COIN_MASK)
                starVisibility[2] = false;
        }
    }

    // Newer: Disable stars for the unused Cutland map
    if (world->mLevelInfoID == 15) {
        starVisibility[0] = false;
        starVisibility[1] = false;
        starVisibility[2] = false;
    }

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

    mpTextBoxes[WorldName]->SetSRTElement(0,  startX + 4.0f);
    mpTextBoxes[WorldNameS]->SetSRTElement(0, startX + 6.0f);
}



void dWMHud_c::enteredNode(dKPNode_s *node) {
    /*if (node == 0)
        node = dScKoopatlas_c::instance->mPathManager.currentNode;

    if (node->type == dKPNode_s::LEVEL && mInitalDispComplete) {*/
        mDispHeader = true;
    //    mpHeaderNode = node;
    //}
}

void dWMHud_c::leftNode() {
    if (mLayout.GetAnimGroup()[ANIM_SHOW_HEADER].mFrameCtrl.mCurrFrame > 0.1f) {
        // not hidden
        if ((mLayout.isAnime(ANIM_SHOW_HEADER) && !(mLayout.GetAnimGroup()[ANIM_SHOW_HEADER].mFrameCtrl.mFlags & 2))
                || (!mLayout.isAnime(ANIM_SHOW_HEADER))) {
            // currently being shown, OR fully shown already
            playHideAnim(ANIM_SHOW_HEADER);
        }
    }
}


void dWMHud_c::hideFooter() {
    if (mFooterVisible)
        playHideAnim(ANIM_SHOW_FOOTER);
}

void dWMHud_c::showFooter() {
    if (!mInitalDispComplete)
        return;
    mDispFooter = true;
    if (mFooterVisible)
        playHideAnim(ANIM_SHOW_FOOTER);
}


void dWMHud_c::setupLives() {
    static const int LogicalPlayerIDs[] = {0,1,3,2};

    mpPicturePanes[P_marioFace_00]->SetVisible(false);
    mpPicturePanes[P_luigiFace_00]->SetVisible(false);
    mpPicturePanes[P_BkinoFace_00]->SetVisible(false);
    mpPicturePanes[P_YkinoFace_00]->SetVisible(false);

    int playerCount = 0;

    for (int i = 0; i < 4; i++) {
        // The part in setupLives()
        int playerID = LogicalPlayerIDs[i];
        int slotID = SearchForIndexOfPlayerID(playerID);
        int lives = daPyMng_c::mRest[slotID];
        int length = 2;

        dGameCom::LayoutDispNumber(lives, length, mpTextBoxes[T_lifeNumber_00+slotID], true);

        // The part in setupIconThings()
        if (dGameCom::PlayerEnterCheck(slotID)) {
            playerCount++;

            nw4r::lyt::Pane *facePane = mpPicturePanes[P_marioFace_00+playerID];
            facePane->SetTranslate(mpNullPanes[playerCount-1]->GetTranslate());
            facePane->SetVisible(true);
        }
    }

    for (int i = 0; i < 4; i++)
        mpNullPanes[i]->SetVisible(false);
    mpNullPanes[playerCount-1]->SetVisible(true);
}

void dWMHud_c::controllerConnectCheck() {
    int type = dGameKey_c::m_instance->mRemocon[0]->mAttachedExtension;

    if (type != mControllerType) {
        mControllerType = type;

        dInfo_c::m_instance->mExtensionAttached = (type == 0) ? 0 : 1;

        MsgRes_c *msgRes = dMessage_c::getMesRes();
        mpTextBoxes[ItemsButtonInfo]->setMessage(msgRes, BMG_CATEGORY_COURSE_SELECT_GUIDE, MSG_CS_ITEMS, 0);
    }
}
#endif