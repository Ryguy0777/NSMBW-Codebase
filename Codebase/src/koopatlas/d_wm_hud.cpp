#include <kamek.h>
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_wm_hud.hpp>
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
        mLayout.mClipSettings.mEnabled = true;
        mLayout.mClipSettings.mPos.x = 0;
        mLayout.mClipSettings.mPos.y = 52;
        mLayout.mClipSettings.mSize.x = 640;
        mLayout.mClipSettings.mSize.y = 352;
    }

    mLayout.AllAnimeEndSetup();
    mLayout.AnimeStartSetup(ANIM_SHOW_LIVES, false);

    // are these needed?
    //layout.resetAnim(SHOW_FOOTER);
    //layout.resetAnim(SHOW_HEADER);
    //layout.resetAnim(HIDE_ALL);

    mHeaderCol.setTexMap(mpPicturePanes[Header_Right]->GetMaterial()->GetTexMapAry());
    mHeaderCol.applyAlso(mpPicturePanes[Header_Centre]->GetMaterial()->GetTexMapAry());
    mFooterCol.setTexMap(mpPicturePanes[Footer]->GetMaterial()->GetTexMapAry());

    //test, remove once header/footer info is done
    mHeaderCol.colourise(330, 74, -8);
    mFooterCol.colourise(330, 74, -8);

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
    //mDispFooter = (save->newerWorldName[0] != 0) && (save->hudHintH != 2000);

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

    //int scCount = getUnspentStarCoinCount();
    //WriteNumberToTextBox(&scCount, StarCoinCounter, false);

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
    /*dLevelInfo_c *levelInfo = &dLevelInfo_c::s_info;

    dLevelInfo_c::entry_s *infEntry = levelInfo->searchBySlot(
            nodeForHeader->levelNumber[0]-1, nodeForHeader->levelNumber[1]-1);

    if (infEntry == 0) {
        LevelName->SetString(L"Unknown Level Name!");
        LevelNameS->SetString(L"Unknown Level Name!");
        return;
    }

    // LEVEL NAME
    wchar_t convertedLevelName[100];
    const char *sourceLevelName = levelInfo->getNameForLevel(infEntry);
    int charCount = 0;
    
    while (*sourceLevelName != 0 && charCount < 99) {
        convertedLevelName[charCount] = *sourceLevelName;
        sourceLevelName++;
        charCount++;
    }
    convertedLevelName[charCount] = 0;

    LevelName->SetString(convertedLevelName);
    LevelNameS->SetString(convertedLevelName);

    // a hack because I don't feel like editing the rlyt
    LevelName->size.x = LevelNameS->size.x = 400.0f;

    // LEVEL NUMBER
    wchar_t levelNumber[16];
    getNewerLevelNumberString(infEntry->displayWorld, infEntry->displayLevel, levelNumber);

    LevelNumber->SetString(levelNumber);

    // make the picture shadowy
    int sidx = 0;
    while (levelNumber[sidx]) {
        if (levelNumber[sidx] == 11) {
            levelNumber[sidx+1] = 0x200 | (levelNumber[sidx+1]&0xFF);
            sidx += 2;
        }
        sidx++;
    }
    LevelNumberS->SetString(levelNumber);

    nw4r::ut::TextWriter tw2;
    tw2.font = LevelNumber->font;
    tw2.SetFontSize(LevelNumber->fontSizeX, LevelNumber->fontSizeY);
    tw2.lineSpace = LevelNumber->lineSpace;
    tw2.charSpace = LevelNumber->charSpace;
    if (LevelNumber->tagProc != 0)
        tw2.tagProcessor = LevelNumber->tagProc;

    float currentPos = tw2.CalcStringWidth(levelNumber, wcslen(levelNumber));
    currentPos += LevelNumber->trans.x + 12.0f;

    // INFO
    int w = nodeForHeader->levelNumber[0] - 1;
    int l = nodeForHeader->levelNumber[1] - 1;

    u32 conds = GetSaveFile()->GetBlock(-1)->GetLevelCondition(w, l);
    // States: 0 = invisible, 1 = visible, 2 = faded
    int normalState = 0, secretState = 0;

    if ((conds & COND_BOTH_EXITS) && (infEntry->flags & 0x30) == 0x30) {
        // If this level has two exits and one of them is already collected,
        // then show the faded flags
        normalState = 2;
        secretState = 2;
    }

    if ((conds & COND_NORMAL) && (infEntry->flags & 0x10))
        normalState = 1;
    if ((conds & COND_SECRET) && (infEntry->flags & 0x20))
        secretState = 1;

    NormalExitFlag->trans.x = currentPos;
    NormalExitFlag->alpha = (normalState == 2) ? 80 : 255;
    NormalExitFlag->SetVisible(normalState > 0);
    if (normalState > 0)
        currentPos += NormalExitFlag->size.x;

    SecretExitFlag->trans.x = currentPos;
    SecretExitFlag->alpha = (secretState == 2) ? 80 : 255;
    SecretExitFlag->SetVisible(secretState > 0);
    if (secretState > 0)
        currentPos += SecretExitFlag->size.x;

    // are star coins enabled or not?
    bool haveSC = (infEntry->flags & 2);

    for (int i = 0; i < 3; i++) {
        bool flag = (conds & (COND_COIN1 << i));
        StarCoinOn[i]->SetVisible(flag);
        StarCoinOff[i]->SetVisible(haveSC);
        if (haveSC) {
            StarCoinOff[i]->trans.x = currentPos;
            currentPos += StarCoinOff[i]->size.x + 4.0f;
        }
    }

    // SIZE THING
    nw4r::ut::TextWriter tw;
    tw.font = LevelName->font;
    tw.SetFontSize(LevelName->fontSizeX, LevelName->fontSizeY);
    tw.lineSpace = LevelName->lineSpace;
    tw.charSpace = LevelName->charSpace;
    if (LevelName->tagProc != 0)
        tw.tagProcessor = LevelName->tagProc;

    float width = tw.CalcStringWidth(convertedLevelName, charCount);
    float totalWidth = width + LevelName->trans.x - 20.0f;
    if (totalWidth < currentPos)
        totalWidth = currentPos;
    Header_Centre->size.x = totalWidth;
    Header_Right->trans.x = totalWidth;

    SaveBlock *save = GetSaveFile()->GetBlock(-1);
    mHeaderCol.colourise(save->hudHintH%1000, save->hudHintS, save->hudHintL);*/
}


void dWMHud_c::loadFooterInfo() {
    /*SaveBlock *save = GetSaveFile()->GetBlock(-1);

    wchar_t convertedWorldName[32];
    int i;
    for (i = 0; i < 32; i++) {
        convertedWorldName[i] = save->newerWorldName[i];
        if (convertedWorldName[i] == 0)
            break;
    }
    convertedWorldName[31] = 0;

    WorldName->SetString(convertedWorldName);
    WorldNameS->SetString(convertedWorldName);

    WorldName->colour1 = save->hudTextColours[0];
    WorldName->colour2 = save->hudTextColours[1];

    mFooterCol.colourise(save->hudHintH%1000, save->hudHintS, save->hudHintL);

    // figure out if stars are needed
    // Star 0: world is complete
    // Star 1: all exits complete
    // Star 2: all star coins obtained
    
    bool starVisibility[3];
    starVisibility[0] = false;

    dLevelInfo_c *linfo = &dLevelInfo_c::s_info;
    dLevelInfo_c::entry_s *lastLevel = linfo->searchByDisplayNum(save->newerWorldID, lastLevelIDs[save->newerWorldID]);
    if (lastLevel) {
        starVisibility[0] = (save->GetLevelCondition(lastLevel->worldSlot,lastLevel->levelSlot) & COND_NORMAL);
    }

    // now calculate the other two
    starVisibility[1] = true;
    starVisibility[2] = true;

    dLevelInfo_c::section_s *sect = linfo->getSectionByIndex(save->newerWorldID);

    for (int i = 0; i < sect->levelCount; i++) {
        dLevelInfo_c::entry_s *entry = &sect->levels[i];

        if (entry->flags & 2) {
            u32 conds = save->GetLevelCondition(entry->worldSlot, entry->levelSlot);

            if (((entry->flags & 0x10) && !(conds & COND_NORMAL)) ||
                    ((entry->flags & 0x20) && !(conds & COND_SECRET)))
                        starVisibility[1] = false;

            if ((conds & COND_COIN_ALL) != COND_COIN_ALL)
                starVisibility[2] = false;
        }
    }

    if (save->newerWorldID == 15) {
        starVisibility[0] = false;
        starVisibility[1] = false;
        starVisibility[2] = false;
    }

    float startX = Star[0]->trans.x;
    for (int i = 0; i < 3; i++) {
        Star[i]->SetVisible(starVisibility[i]);
        Star[i]->trans.x = startX;
        if (starVisibility[i]) {
            startX += Star[i]->size.x + 4.0f;
        }
    }

    WorldName->trans.x = startX + 4.0f;
    WorldNameS->trans.x = startX + 6.0f;*/
}



void dWMHud_c::enteredNode(dKPNode_s *node) {
    /*if (node == 0)
        node = dScKoopatlas_c::instance->mPathManager.currentNode;

    if (node->type == dKPNode_s::LEVEL && mInitalDispComplete) {
        mDispHeader = true;
        mpHeaderNode = node;
    }*/
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

        //int beef = (type == 0) ? 0 : 1;
        dInfo_c::m_instance->mExtensionAttached = (type == 0) ? 0 : 1;

        MsgRes_c *msgRes = dMessage_c::getMesRes();
        mpTextBoxes[ItemsButtonInfo]->setMessage(msgRes, BMG_CATEGORY_COURSE_SELECT_GUIDE, MSG_CS_ITEMS, 0);
    }
}
#endif