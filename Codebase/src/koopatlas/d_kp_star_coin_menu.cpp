#include <kamek.h>
#include <new/game_config.h>

#if defined(KOOPATLAS_DEV_ENABLED) || defined(NEWER_STAR_COINS_MENU)
#include <constants/sound_list.h>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_game_key.hpp>
#include <game/bases/d_save_mng.hpp>
#include <new/bases/koopatlas/d_kp_star_coin_menu.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>

const int dKpStarCoinMenu_c::sc_secretCode[] = {
    WPAD_BUTTON_RIGHT,
    WPAD_BUTTON_RIGHT,
    WPAD_BUTTON_LEFT,
    WPAD_BUTTON_LEFT,
    WPAD_BUTTON_UP,
    WPAD_BUTTON_DOWN,
    WPAD_BUTTON_UP,
    WPAD_BUTTON_DOWN,
    WPAD_BUTTON_1,
    WPAD_BUTTON_2,
    0, // Used to tell us that the code is complete
};

// List of valid keys in the secret code sequence. Used to determine if the current press should count towards the code
const int dKpStarCoinMenu_c::sc_secretKeys = (WPAD_BUTTON_RIGHT|WPAD_BUTTON_LEFT|WPAD_BUTTON_UP|WPAD_BUTTON_DOWN|WPAD_BUTTON_1|WPAD_BUTTON_2);

// TEMP: These would be externs if properly implemented
static bool enableHardMode;
static bool enableDebugMode;
static u8 isReplayEnabled;

dKpStarCoinMenu_c *dKpStarCoinMenu_c_classInit() {
    return new dKpStarCoinMenu_c;
}

// Replace COLLECTION_COIN actor
kmWritePointer(0x8093F810, &dKpStarCoinMenu_c_classInit);

dKpStarCoinMenu_c::dKpStarCoinMenu_c() : mStateMgr(*this, StateID_Initial) {
    mHasLayoutLoaded = false;
    mIsVisible = false;
}

STATE_DEFINE(dKpStarCoinMenu_c, Initial);
STATE_DEFINE(dKpStarCoinMenu_c, ShowWait);
STATE_DEFINE(dKpStarCoinMenu_c, ShowSectionWait);
STATE_DEFINE(dKpStarCoinMenu_c, Wait);
STATE_DEFINE(dKpStarCoinMenu_c, HideSectionWait);
STATE_DEFINE(dKpStarCoinMenu_c, HideWait);

int dKpStarCoinMenu_c::create() {
    bool res = mLayout.ReadResource("starCoins.arc", false);
    if (!res) {
        return NOT_READY;
    }

    mLayout.build("starCoins_00.brlyt", nullptr);

    mpRootPane = mLayout.getRootPane();

    mpRootPane->SetSRTElement(0, -112.0f);
    if (!dGameCom::GetAspectRatio()) {
        mpRootPane->SetSRTElement(6, 19.0f/26.0f);
        mpRootPane->SetSRTElement(7, 352.0f/456.0f);
        mLayout.mScissorMask.mEnabled = true;
        mLayout.mScissorMask.mPos.x = 0;
        mLayout.mScissorMask.mPos.y = 52;
        mLayout.mScissorMask.mSize.x = 640;
        mLayout.mScissorMask.mSize.y = 352;
    }

    static const char *AnmNameTbl[] = {
        "starCoins_00_Show.brlan",
        "starCoins_00_ShowSection.brlan",
        "starCoins_00_HideSection.brlan",
        "starCoins_00_ShowArrow.brlan",
        "starCoins_00_HideArrow.brlan"
    };

    static const int ANIME_INDEX_TBL[] = {0, 1, 2, 3, 4, 3, 4};
    static const char *GROUP_NAME_DT[] = {
        "base",
        "section",
        "section",
        "leftArrow", "leftArrow",
        "rightArrow", "rightArrow"
    };

    static const char *T_PANE_NAME_TBL[] = {
        "LeftTitle", "RightTitle",
        "TotalCoinCount", "UnspentCoinCount",
        "EarnedCoinCount", "EarnedCoinMax",
        "BtnBackText"
    };

    static const char *P_PANE_NAME_TBL[] = {
        "DPadLeft",
        "DPadRight"
    };

    static const char *T_PANE_FIXED_NAME_TBL[] = {
        "TotalCoinsTitle",
        "CollectedTitle",
        "UnspentTitle",
        "BtnWorldSelText",
        "BtnBackText",
        "EarnedCoinsTitle"
    };

    static const int MESSAGE_DATA_TBL[] = {
        0x10,
        0x11,
        0x12,
        0x13,
        0x14,
        0x15
    };

    mLayout.AnimeResRegister(AnmNameTbl, ARRAY_SIZE(AnmNameTbl));
    mLayout.GroupRegister(GROUP_NAME_DT, ANIME_INDEX_TBL, ARRAY_SIZE(GROUP_NAME_DT));
    mLayout.TPaneRegister(T_PANE_NAME_TBL, mpTextBoxes, ARRAY_SIZE(T_PANE_NAME_TBL));
    mLayout.PPaneRegister(P_PANE_NAME_TBL, mpPicturePanes, ARRAY_SIZE(P_PANE_NAME_TBL));
    mLayout.TPaneNameRegister(T_PANE_FIXED_NAME_TBL, MESSAGE_DATA_TBL, BMG_CATEGORY_KOOPATLAS, ARRAY_SIZE(T_PANE_FIXED_NAME_TBL));
    mLayout.AllAnimeEndSetup();

    mpRootPane->SetVisible(false);
    mLayout.mDrawOrder = 1;

    for (int col = 0; col < COLUMN_COUNT; col++) {
        for (int shine = 0; shine < SHINE_COUNT; shine++) {
            char name[8];
            sprintf(name, "Shine%d%d", col, shine);
            mpShines[col][shine] = mLayout.findPictureByName(name);
        }

        for (int row = 0; row < ROW_COUNT; row++) {
            char lname[12];
            sprintf(lname, "LevelName%d%d", col, row);
            mpLevelNames[col][row] = (LytTextBox_c*)mLayout.findTextBoxByName(lname);

            char coname[16], cname[8];
            for (int i = 0; i < 3; i++) {
                sprintf(coname, "CoinOutline%d%d%d", col, row, i);
                mpCoinOutlines[col][row][i] = mLayout.findPictureByName(coname);

                sprintf(cname, "Coin%d%d%d", col, row, i);
                mpCoins[col][row][i] = mLayout.findPictureByName(cname);
            }
        }
    }

    mpPicturePanes[DPadLeft]->SetVisible(false);
    mpPicturePanes[DPadRight]->SetVisible(false);

    mHasLayoutLoaded = true;
    return SUCCEEDED;
}

int dKpStarCoinMenu_c::doDelete() {
    return mLayout.doDelete();
}

int dKpStarCoinMenu_c::execute() {
    if (mIsVisible) {
        mStateMgr.executeState();
        mLayout.AnimePlay();
        mLayout.calc();
    }
    return SUCCEEDED;
}

int dKpStarCoinMenu_c::draw() {
    if (mIsVisible) {
        mLayout.entry();
    }
    return SUCCEEDED;
}

void dKpStarCoinMenu_c::dispMenu(int worldNum) {
    if (mStateMgr.getStateID() == &StateID_Initial) {
        mCurrWorld = worldNum;
        mIsVisible = true;
    }
}

bool dKpStarCoinMenu_c::canScrollLeft() const {
    return (mCurrWorldIdx > 0);
}

bool dKpStarCoinMenu_c::canScrollRight() const {
    return (mCurrWorldIdx < (mOpenWorldNum-1));
}

void dKpStarCoinMenu_c::leftArrowDisp(bool value) {
    if (value) {
        if (!mLeftArrowActive) {
            mLeftArrowActive = true;
            mLayout.AnimeStartSetup(ANIM_SHOW_LEFT_ARROW, false);
            mpPicturePanes[DPadLeft]->SetVisible(true);
        }
    } else {
        if (mLeftArrowActive) {
            mLeftArrowActive = false;
            mLayout.AnimeStartSetup(ANIM_HIDE_LEFT_ARROW, false);
            mpPicturePanes[DPadLeft]->SetVisible(false);
        }
    }
}

void dKpStarCoinMenu_c::rightArrowDisp(bool value) {
    if (value) {
        if (!mRightArrowActive) {
            mRightArrowActive = true;
            mLayout.AnimeStartSetup(ANIM_SHOW_RIGHT_ARROW, false);
            mpPicturePanes[DPadRight]->SetVisible(true);
        }
    } else {
        if (mRightArrowActive) {
            mRightArrowActive = false;
            mLayout.AnimeStartSetup(ANIM_HIDE_RIGHT_ARROW, false);
            mpPicturePanes[DPadRight]->SetVisible(false);
        }
    }
}

void dKpStarCoinMenu_c::loadMenuInfo() {
    int unspentCoins = dGameCom::getUnspentStarCoinCount();
    int coins = dGameCom::getStarCoinCount();

    dGameCom::LayoutDispNumberDigit(unspentCoins, mpTextBoxes[UnspentCoinCount], false);
    dGameCom::LayoutDispNumberDigit(coins, mpTextBoxes[TotalCoinCount], false);

    mCurrWorldIdx = -1;
    mOpenWorldNum = 0;

    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
#ifdef KOOPATLAS_DEV_ENABLED
    dWorldInfo_c::world_s *world = dWorldInfo_c::m_instance.getWorld(save->mWorldInfoIdx);
    int wantedSection = world->mLevelInfoID-1;
#else
    int wantedSection = mCurrWorld;
#endif

    // Figure out which sections should be available
    for (int i = 0; i < dLevelInfo_c::m_instance.sectionCount(); i++) {
        dLevelInfo_c::section_s *section = dLevelInfo_c::m_instance.getSection(i);

        bool hasLevels = false;
        for (int j = 0; j < section->mLevelCount; j++) {
            dLevelInfo_c::entry_s *level = &section->mLevels[j];
            if (level->mFlag & dLevelInfo_c::FLAG_VALID_LEVEL) {
                if (save->isCourseDataFlag(level->mWorldSlot, level->mLevelSlot, dMj2dGame_c::GOAL_NORMAL)) {
                    hasLevels = true;
                    break;
                }
            }
        }

        if (hasLevels) {
            if (i == wantedSection) {
                mCurrWorld = wantedSection;
                mCurrWorldIdx = mOpenWorldNum;
            }
            mWorldIndices[mOpenWorldNum++] = i;
        }
    }

    // If we didn't find the wanted page, use the first one available
    if (mCurrWorldIdx == -1) {
        mCurrWorldIdx = 0;
        mCurrWorld = mWorldIndices[0];
    }
}

// TODO: Add byte to WorldInfo that contains the world idx of the related subworld
void dKpStarCoinMenu_c::loadSectionInfo() {
    dLevelInfo_c::entry_s *visibleLevels[COLUMN_COUNT][ROW_COUNT];

    // Reset everything
    for (int i = 0; i < COLUMN_COUNT; i++) {
        for (int j = 0; j < SHINE_COUNT; j++) {
            mpShines[i][j]->SetVisible(false);
        }

        for (int j = 0; j < ROW_COUNT; j++) {
            visibleLevels[i][j] = nullptr;
            mpLevelNames[i][j]->SetVisible(false);

            for (int k = 0; k < 3; k++) {
                mpCoinOutlines[i][j][k]->SetVisible(false);
                mpCoins[i][j][k]->SetVisible(false);
            }
        }
    }

    // Get everything we'll need
    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    dLevelInfo_c *linfo = &dLevelInfo_c::m_instance;

    dLevelInfo_c::entry_s *names[COLUMN_COUNT];
    for (int i = 0; i < COLUMN_COUNT; i++) {
        names[i] = linfo->getEntryFromDispID(mCurrWorld, 100+i);
    }

    bool usesSubworlds = (COLUMN_COUNT > 1) && names[1];

    int currentPosition[COLUMN_COUNT];
    int currentColumn = 0; // Only incremented in single-subworld mode

    for (int i = 0; i < COLUMN_COUNT; i++) {
        currentPosition[i] = 0;
    }

    dLevelInfo_c::section_s *section = linfo->getSection(mCurrWorld);

    int collectedCoins = 0, totalCoins = 0;

    for (int i = 0; i < section->mLevelCount; i++) {
        dLevelInfo_c::entry_s *level = &section->mLevels[i];

        // Only pay attention to real levels
        if (!(level->mFlag & dLevelInfo_c::FLAG_VALID_LEVEL)) {
            continue;
        }

        totalCoins += 3;

        // Is this level unlocked?
        u32 conds = save->getCourseDataFlag(level->mWorldSlot, level->mLevelSlot);
        if (!(conds & 0x200)) {
        	continue;
        }

        // Give it a slot
        if (usesSubworlds) {
            currentColumn = (level->mFlag & dLevelInfo_c::FLAG_SECOND_HALF) ? 1 : 0;
        } else {
            if (currentPosition[currentColumn] >= ROW_COUNT) {
                currentColumn++;
            }
        }

        visibleLevels[currentColumn][currentPosition[currentColumn]++] = level;
    }

    // If the first column is empty, move the second one over
    if (currentPosition[0] == 0 && usesSubworlds) {
        for (int i = 0; i < currentPosition[1]; i++) {
            visibleLevels[0][i] = visibleLevels[1][i];
            visibleLevels[1][i] = nullptr;
        }

        names[0] = names[1];
        names[1] = nullptr;
    }

    // If the second column is empty, remove its name
    if ((currentPosition[1] == 0) && usesSubworlds) {
        names[1] = nullptr;
    }

    // Set the names
    const wchar_t *leftName = getWorldName(mCurrWorld);
    mpTextBoxes[LeftTitle]->SetString(leftName, 0);
    //if (names[1] != nullptr) {
    //	WriteAsciiToTextBox(RightTitle, linfo->getLevelName(names[1]));
    //}
    mpTextBoxes[RightTitle]->SetVisible(names[1] != nullptr);

    // Write all level data
    for (int col = 0; col < COLUMN_COUNT; col++) {
        for (int row = 0; row < ROW_COUNT; row++) {
            dLevelInfo_c::entry_s *level = visibleLevels[col][row];
            if (level == nullptr) {
                continue;
            }

            u32 conds = save->getCourseDataFlag(level->mWorldSlot, level->mLevelSlot);

            // Show shines for levels
            if (!(row & 1)) {
                int shineID = row / 2;
                if (shineID < SHINE_COUNT) {
                    mpShines[col][shineID]->SetVisible(true);
                }
            }

            // Star Coins
            for (int coin = 0; coin < 3; coin++) {
                mpCoinOutlines[col][row][coin]->SetVisible(true);

                if (conds & (dMj2dGame_c::COIN1_COLLECTED << coin)) {
                    // Hide outlines so they aren't seen during animations
                    mpCoinOutlines[col][row][coin]->SetVisible(false);
                    mpCoins[col][row][coin]->SetVisible(true);
                    collectedCoins++;
                }
            }

            mpLevelNames[col][row]->SetVisible(true);
            const wchar_t *levelName = getLevelName(level->mDisplayWorld, level->mDisplayLevel);
            mpLevelNames[col][row]->SetString(levelName, 0);
        }
    }

    // Write coin counts
    dGameCom::LayoutDispNumberDigit(collectedCoins, mpTextBoxes[EarnedCoinCount], false);
    dGameCom::LayoutDispNumberDigit(totalCoins, mpTextBoxes[EarnedCoinMax], false);
}

void dKpStarCoinMenu_c::dispSecretMessage(int titleMsg, int bodyMsgStart, int bodyMsgCount, int bodyMsgStart2, int bodyMsgCount2) {
    MsgRes_c *msgRes = dMessage_c::getMesRes();
    mpTextBoxes[LeftTitle]->SetVisible(true);
    mpTextBoxes[RightTitle]->SetVisible(false);

    const wchar_t *titleStr = msgRes->getMsg(BMG_CATEGORY_KOOPATLAS, titleMsg);
    mpTextBoxes[LeftTitle]->SetString(titleStr, 0);

    // Hide level data
    for (int c = 0; c < COLUMN_COUNT; c++) {
        for (int i = 0; i < SHINE_COUNT; i++) {
            mpShines[c][i]->SetVisible(false);
        }
        for (int r = 0; r < ROW_COUNT; r++) {
            mpLevelNames[c][r]->SetVisible(false);

            for (int i = 0; i < 3; i++) {
                mpCoinOutlines[c][r][i]->SetVisible(false);
                mpCoins[c][r][i]->SetVisible(false);
            }
        }
    }

    for (int i = 0; i < bodyMsgCount; i++) {
        mpLevelNames[0][i]->SetVisible(true);
        const wchar_t *lineStr = msgRes->getMsg(BMG_CATEGORY_KOOPATLAS, bodyMsgStart+i);
        mpLevelNames[0][i]->SetString(lineStr, 0);
    }

    if (bodyMsgStart2 != 0) {
        for (int i = 0; i < bodyMsgCount2; i++) {
            mpLevelNames[1][i]->SetVisible(true);
            const wchar_t *lineStr = msgRes->getMsg(BMG_CATEGORY_KOOPATLAS, bodyMsgStart2+i);
            mpLevelNames[1][i]->SetString(lineStr, 0);
        }
    }
}


void dKpStarCoinMenu_c::initializeState_Initial() { }
void dKpStarCoinMenu_c::executeState_Initial() {
    loadMenuInfo();

    mLayout.AnimeStartSetup(ANIM_SHOW_ALL, false);
    mLayout.ReverseAnimeStartSetup(ANIM_SHOW_SECTION, false);
    mLayout.ReverseAnimeStartSetup(ANIM_SHOW_LEFT_ARROW, false);
    mLayout.ReverseAnimeStartSetup(ANIM_SHOW_RIGHT_ARROW, false);

    mpRootPane->SetVisible(true);
    mStateMgr.changeState(StateID_ShowWait);
}
void dKpStarCoinMenu_c::finalizeState_Initial() {
    mSecretCodeIdx = 0;
    mMinusKeyPressIdx = 0;
}


void dKpStarCoinMenu_c::initializeState_ShowWait() { }
void dKpStarCoinMenu_c::executeState_ShowWait() {
    if (!mLayout.isAnime(ANIM_SHOW_ALL)) {
        mStateMgr.changeState(StateID_ShowSectionWait);
    }
}
void dKpStarCoinMenu_c::finalizeState_ShowWait() { }


void dKpStarCoinMenu_c::initializeState_ShowSectionWait() {
    loadSectionInfo();
    mLayout.AnimeStartSetup(ANIM_SHOW_SECTION, false);

    if (canScrollLeft()) {
        leftArrowDisp(true);
    }
    if (canScrollRight()) {
        rightArrowDisp(true);
    }
}
void dKpStarCoinMenu_c::executeState_ShowSectionWait() {
    if (!mLayout.isAnime(ANIM_SHOW_SECTION)) {
        mStateMgr.changeState(StateID_Wait);
    }
}
void dKpStarCoinMenu_c::finalizeState_ShowSectionWait() { }


void dKpStarCoinMenu_c::initializeState_Wait() { }
void dKpStarCoinMenu_c::executeState_Wait() {
    int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;
    int keyPress = WPAD_BUTTON_A | WPAD_BUTTON_PLUS;

    // A and Plus
    if ((dGameKey_c::m_instance->mRemocon[0]->mDownButtons == keyPress) && (pressed & keyPress)) {
        if (!enableHardMode) {
            enableHardMode = true;
            MapReport("Hard Mode enabled!\n");
            SndAudioMgr::sInstance->startSystemSe(SE_VOC_MA_CS_COURSE_IN_HARD, 1);
            dispSecretMessage(0x20, 0x21, 9);
        } else {
            enableHardMode = false;
            MapReport("Hard Mode disabled!\n");
            dispSecretMessage(0x30, 0x31, 2);
        }
        return;
    }

    if (pressed & sc_secretKeys) {
        int nextKey = sc_secretCode[mSecretCodeIdx];
        if (pressed & nextKey) {
            mSecretCodeIdx++;

            // This means we've completed the code
            if (sc_secretCode[mSecretCodeIdx] == 0) {
                mSecretCodeIdx = 0;
                SndAudioMgr::sInstance->startSystemSe(SE_VOC_MA_THANK_YOU, 1);

                if (isReplayEnabled != 100) {
                    isReplayEnabled = 100;
                    MapReport("Replay Recording enabled!\n");
                    dispSecretMessage(0x40, 0x41, 9);
                } else {
                    isReplayEnabled = 0;
                    MapReport("Replay Recording disabled!\n");
                    dispSecretMessage(0x50, 0x51, 2);
                }
            }
            return;
        } else {
            mSecretCodeIdx = 0;
        }
    }

    if (pressed & WPAD_BUTTON_MINUS) {
        mMinusKeyPressIdx++;

        if (mMinusKeyPressIdx >= 16) {
            mMinusKeyPressIdx = 0;

            enableDebugMode = !enableDebugMode;
            if (enableDebugMode) {
                SndAudioMgr::sInstance->startSystemSe(SE_VOC_MA_GET_PRIZE, 1);
                dispSecretMessage(0x60, 0x61, 9, 0x6A, 9);
            } else {
                dispSecretMessage(0x80, 0x81, 6);
            }
        }

    } else if (pressed & WPAD_BUTTON_1) {
        SndAudioMgr::sInstance->startSystemSe(SE_SYS_BACK, 1);
        mWillExit = true;
        mStateMgr.changeState(StateID_HideSectionWait);

    } else if ((pressed & WPAD_BUTTON_UP) && canScrollLeft()) {
        mCurrWorld = mWorldIndices[--mCurrWorldIdx];
        mWillExit = false;
        mStateMgr.changeState(StateID_HideSectionWait);

    } else if ((pressed & WPAD_BUTTON_DOWN) && canScrollRight()) {
        mCurrWorld = mWorldIndices[++mCurrWorldIdx];
        mWillExit = false;
        mStateMgr.changeState(StateID_HideSectionWait);
    }
}
void dKpStarCoinMenu_c::finalizeState_Wait() { }


void dKpStarCoinMenu_c::initializeState_HideSectionWait() {
    mLayout.AnimeStartSetup(ANIM_HIDE_SECTION, false);
    if (mWillExit) {
        leftArrowDisp(false);
        rightArrowDisp(false);
    } else {
        SndAudioMgr::sInstance->startSystemSe(SE_SYS_PAGE, 1);
        leftArrowDisp(canScrollLeft());
        rightArrowDisp(canScrollRight());
    }
}
void dKpStarCoinMenu_c::executeState_HideSectionWait() {
    if (!mLayout.isAnime(ANIM_HIDE_SECTION)) {
        if (mWillExit) {
            mStateMgr.changeState(StateID_HideWait);
        } else {
            mStateMgr.changeState(StateID_ShowSectionWait);
        }
    }
}
void dKpStarCoinMenu_c::finalizeState_HideSectionWait() { }


void dKpStarCoinMenu_c::initializeState_HideWait() {
    mLayout.AnimeStartSetup(ANIM_SHOW_ALL, true);
    mLayout.GetAnimGroup()[ANIM_SHOW_ALL].mFrameCtrl.mFlags = (m2d::FrameCtrl_c::NO_LOOP | m2d::FrameCtrl_c::REVERSE);
}
void dKpStarCoinMenu_c::executeState_HideWait() {
    if (!mLayout.isAnime(ANIM_SHOW_ALL)) {
        mStateMgr.changeState(StateID_Initial);
    }
}
void dKpStarCoinMenu_c::finalizeState_HideWait() {
    mpRootPane->SetVisible(false);
    mIsVisible = false;
}
#endif