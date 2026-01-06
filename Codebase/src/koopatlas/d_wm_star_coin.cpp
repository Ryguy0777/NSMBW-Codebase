#include <kamek.h>
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_wm_star_coin.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>
#include <constants/sound_list.h>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_game_key.hpp>
#include <game/bases/d_save_mng.hpp>

dWMStarCoin_c *dWMStarCoin_c::m_instance = nullptr;

dWMStarCoin_c *dWMStarCoin_c::build() {
    dWMStarCoin_c *c = new dWMStarCoin_c;
    m_instance = c;
    return c;
}

// Replace WM_GHOST actor
kmWritePointer(0x80982D1C, &dWMStarCoin_c::build);

dWMStarCoin_c::dWMStarCoin_c() : mStateMgr(*this, StateID_Hidden) {
    mHasLayoutLoaded = false;
    mIsVisible = false;
}

STATE_DEFINE(dWMStarCoin_c, Hidden);
STATE_DEFINE(dWMStarCoin_c, ShowWait);
STATE_DEFINE(dWMStarCoin_c, ShowSectionWait);
STATE_DEFINE(dWMStarCoin_c, Wait);
STATE_DEFINE(dWMStarCoin_c, HideSectionWait);
STATE_DEFINE(dWMStarCoin_c, HideWait);

int dWMStarCoin_c::create() {
    bool res = mLayout.ReadResource("starCoins.arc", false);
    if (!res) {
        return false;
    }

    mLayout.build("starCoins_00.brlyt", nullptr);

    mLayout.getRootPane()->SetSRTElement(0, -112.0f);
    if (!dGameCom::GetAspectRatio()) {
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
    return true;
}


int dWMStarCoin_c::doDelete() {
    return mLayout.doDelete();
}


void dWMStarCoin_c::show() {
    if (mStateMgr.getStateID() == &StateID_Hidden)
        mStateMgr.changeState(StateID_ShowWait);
}


int dWMStarCoin_c::execute() {
    mStateMgr.executeState();
    if (mIsVisible) {
        mLayout.AnimePlay();
        mLayout.calc();
    }
    return true;
}

int dWMStarCoin_c::draw() {
    if (mIsVisible)
        mLayout.draw();
    return true;
}

void dWMStarCoin_c::showLeftArrow() {
    if (!mLeftArrowActive) {
        mLeftArrowActive = true;
        mLayout.AnimeStartSetup(ANIM_SHOW_LEFT_ARROW, false);
        mpPicturePanes[DPadLeft]->SetVisible(true);
    }
}

void dWMStarCoin_c::showRightArrow() {
    if (!mRightArrowActive) {
        mRightArrowActive = true;
        mLayout.AnimeStartSetup(ANIM_SHOW_RIGHT_ARROW, false);
        mpPicturePanes[DPadRight]->SetVisible(true);
    }
}

void dWMStarCoin_c::hideLeftArrow() {
    if (mLeftArrowActive) {
        mLeftArrowActive = false;
        mLayout.AnimeStartSetup(ANIM_HIDE_LEFT_ARROW, false);
        mpPicturePanes[DPadLeft]->SetVisible(false);
    }
}

void dWMStarCoin_c::hideRightArrow() {
    if (mRightArrowActive) {
        mRightArrowActive = false;
        mLayout.AnimeStartSetup(ANIM_HIDE_RIGHT_ARROW, false);
        mpPicturePanes[DPadRight]->SetVisible(false);
    }
}

void dWMStarCoin_c::setLeftArrowVisible(bool value) {
    if (value)
        showLeftArrow();
    else
        hideLeftArrow();
}

void dWMStarCoin_c::setRightArrowVisible(bool value) {
    if (value)
        showRightArrow();
    else
        hideRightArrow();
}

bool dWMStarCoin_c::canScrollLeft() const {
    return (mCurrentWorldIndex > 0);
}
bool dWMStarCoin_c::canScrollRight() const {
    return (mCurrentWorldIndex < (mOpenWorldCount-1));
}

void dWMStarCoin_c::loadInfo() {
    int unspentCoins = dGameCom::getUnspentStarCoinCount();
    int coins = dGameCom::getStarCoinCount();

    dGameCom::LayoutDispNumberDigit(unspentCoins, mpTextBoxes[UnspentCoinCount], false);
    dGameCom::LayoutDispNumberDigit(coins, mpTextBoxes[TotalCoinCount], false);

    mCurrentWorld = -1;
    mCurrentWorldIndex = -1;
    mOpenWorldCount = 0;

    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    dWorldInfo_c::world_s *world = dWorldInfo_c::m_instance.getWorld(save->mWorldInfoIdx);
    int wantedSection = world->mLevelInfoID-1;

    // Figure out which sections should be available
    for (int i = 0; i < dLevelInfo_c::m_instance.sectionCount(); i++) {
        dLevelInfo_c::section_s *section = dLevelInfo_c::m_instance.getSection(i);

        bool hasLevels = false;
        for (int j = 0; j < section->mLevelCount; j++) {
            dLevelInfo_c::entry_s *level = &section->mLevels[j];
            if (level->mFlag & 2) {
                if (save->isCourseDataFlag(level->mWorldSlot, level->mLevelSlot, dMj2dGame_c::GOAL_NORMAL)) {
                    hasLevels = true;
                    break;
                }
            }
        }

        if (hasLevels) {
            if (i == wantedSection) {
                mCurrentWorld = wantedSection;
                mCurrentWorldIndex = mOpenWorldCount;
            }
            mWorldIndices[mOpenWorldCount++] = i;
        }
    }

    // If we didn't find the wanted one, use the first one available
    if (mCurrentWorldIndex == -1) {
        mCurrentWorldIndex = 0;
        mCurrentWorld = mWorldIndices[0];
    }
}

// TODO: Add byte to WorldInfo that contains the world idx of the related subworld
void dWMStarCoin_c::loadSectionInfo() {
    dLevelInfo_c::entry_s *visibleLevels[COLUMN_COUNT][ROW_COUNT];

    // Reset everything
    for (int i = 0; i < COLUMN_COUNT; i++) {
        for (int j = 0; j < SHINE_COUNT; j++)
            mpShines[i][j]->SetVisible(false);

        for (int j = 0; j < ROW_COUNT; j++) {
            visibleLevels[i][j] = 0;

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
        names[i] = linfo->getEntryFromDispID(mCurrentWorld, 100+i);
    }

    bool usesSubworlds = (COLUMN_COUNT > 1) && names[1];

    int currentPosition[COLUMN_COUNT];
    int currentColumn = 0; // Only incremented in single-subworld mode

    for (int i = 0; i < COLUMN_COUNT; i++) {
        currentPosition[i] = 0;
    }

    dLevelInfo_c::section_s *section = linfo->getSection(mCurrentWorld);

    int collectedCoins = 0, totalCoins = 0;
    // collectedCoins is calculated later

    for (int i = 0; i < section->mLevelCount; i++) {
        dLevelInfo_c::entry_s *level = &section->mLevels[i];

        // Only pay attention to real levels
        if (!(level->mFlag & dLevelInfo_c::FLAG_VALID_LEVEL)) {
            continue;
        }

        totalCoins += 3;

        // Is this level unlocked?
        u32 conds = save->getCourseDataFlag(level->mWorldSlot, level->mLevelSlot);

        // Newer-exclusive condition
        //if (!(conds & 0x200))
        //	continue;

        // Give it a slot
        if (usesSubworlds) {
            currentColumn = (level->mFlag & dLevelInfo_c::FLAG_SECOND_HALF) ? 1 : 0;
        } else {
            if (currentPosition[currentColumn] >= ROW_COUNT)
                currentColumn++;
        }

        visibleLevels[currentColumn][currentPosition[currentColumn]++] = level;
    }

    // If the first column is empty, move the second one over
    if (currentPosition[0] == 0 && usesSubworlds) {
        for (int i = 0; i < currentPosition[1]; i++) {
            visibleLevels[0][i] = visibleLevels[1][i];
            visibleLevels[1][i] = 0;
        }

        names[0] = names[1];
        names[1] = nullptr;
    }

    // if the second column is empty, remove its name
    if (currentPosition[1] == 0 && usesSubworlds)
        names[1] = nullptr;

    // Set the names
    const wchar_t *leftName = getWorldName(mCurrentWorld);
    mpTextBoxes[LeftTitle]->SetString(leftName, 0);
    //if (names[1])
    //	WriteAsciiToTextBox(RightTitle, linfo->getLevelName(names[1]));
    mpTextBoxes[RightTitle]->SetVisible(names[1] != 0);

    // load all level info
    for (int col = 0; col < COLUMN_COUNT; col++) {
        for (int row = 0; row < ROW_COUNT; row++) {
            dLevelInfo_c::entry_s *level = visibleLevels[col][row];
            if (!level) {
                continue;
            }

            u32 conds = save->getCourseDataFlag(level->mWorldSlot, level->mLevelSlot);

            if (!(row & 1)) {
                int shineID = row / 2;
                if (shineID < SHINE_COUNT)
                    mpShines[col][shineID]->SetVisible(true);
            }

            for (int coin = 0; coin < 3; coin++) {
                mpCoinOutlines[col][row][coin]->SetVisible(true);

                if (conds & (dMj2dGame_c::COIN1_COLLECTED << coin)) {
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

void dWMStarCoin_c::showSecretMessage(int titleMsg, int bodyMsgStart, int bodyMsgCount, int bodyMsgStart2, int bodyMsgCount2) {
    MsgRes_c *msgRes = dMessage_c::getMesRes();
    mpTextBoxes[LeftTitle]->SetVisible(true);
    mpTextBoxes[RightTitle]->SetVisible(false);

    const wchar_t *titleStr = msgRes->getMsg(BMG_CATEGORY_KOOPATLAS, titleMsg);
    mpTextBoxes[LeftTitle]->SetString(titleStr, 0);

    // Hide the levelinfo
    for (int c = 0; c < COLUMN_COUNT; c++) {
        for (int i = 0; i < SHINE_COUNT; i++)
            mpShines[c][i]->SetVisible(false);
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

static const int secretCode[] = {
    WPAD_BUTTON_RIGHT,WPAD_BUTTON_RIGHT,WPAD_BUTTON_LEFT,WPAD_BUTTON_LEFT,
    WPAD_BUTTON_UP,WPAD_BUTTON_DOWN,WPAD_BUTTON_UP,WPAD_BUTTON_DOWN,
    WPAD_BUTTON_1,WPAD_BUTTON_2,0
};
static const int secretCodeButtons = WPAD_BUTTON_RIGHT|WPAD_BUTTON_LEFT|WPAD_BUTTON_UP|WPAD_BUTTON_DOWN|WPAD_BUTTON_1|WPAD_BUTTON_2;
static int secretCodeIndex = 0;
static int minusCount = 0;
//extern bool enableHardMode;
//extern bool enableDebugMode;
//extern u8 isReplayEnabled;
static bool enableHardMode;
static bool enableDebugMode;
static u8 isReplayEnabled;



void dWMStarCoin_c::initializeState_Hidden() { }
void dWMStarCoin_c::executeState_Hidden() { }
void dWMStarCoin_c::finalizeState_Hidden() { }



void dWMStarCoin_c::initializeState_ShowWait() {
    mIsVisible = true;
    loadInfo();
    mLayout.AnimeStartSetup(ANIM_SHOW_ALL, false);
    mLayout.ReverseAnimeStartSetup(ANIM_SHOW_SECTION, false);
    mLayout.ReverseAnimeStartSetup(ANIM_SHOW_LEFT_ARROW, false);
    mLayout.ReverseAnimeStartSetup(ANIM_SHOW_RIGHT_ARROW, false);
    SndAudioMgr::sInstance->startSystemSe(SE_SYS_DIALOGUE_IN, 1);

    secretCodeIndex = 0;
    minusCount = 0;
}
void dWMStarCoin_c::executeState_ShowWait() {
    if (!mLayout.isAnime(ANIM_SHOW_ALL))
        mStateMgr.changeState(StateID_ShowSectionWait);
}
void dWMStarCoin_c::finalizeState_ShowWait() { }



void dWMStarCoin_c::initializeState_ShowSectionWait() {
    loadSectionInfo();
    mLayout.AnimeStartSetup(ANIM_SHOW_SECTION, false);

    if (canScrollLeft())
        showLeftArrow();
    if (canScrollRight())
        showRightArrow();
}
void dWMStarCoin_c::executeState_ShowSectionWait() {
    if (!mLayout.isAnime(ANIM_SHOW_SECTION))
        mStateMgr.changeState(StateID_Wait);
}
void dWMStarCoin_c::finalizeState_ShowSectionWait() { }



void dWMStarCoin_c::initializeState_Wait() { }
void dWMStarCoin_c::executeState_Wait() {
    int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;

    // A and Plus
    if ((dGameKey_c::m_instance->mRemocon[0]->mDownButtons == 0x810) && (pressed & 0x810)) {
        if (!enableHardMode) {
            enableHardMode = true;
            MapReport("Hard Mode enabled!\n");
            SndAudioMgr::sInstance->startSystemSe(SE_VOC_MA_CS_COURSE_IN_HARD, 1);
            showSecretMessage(0x20, 0x21, 9);
        } else {
            enableHardMode = false;
            MapReport("Hard Mode disabled!\n");
            showSecretMessage(0x30, 0x31, 2);
        }
        return;
    }

    if (pressed & secretCodeButtons) {
        int nextKey = secretCode[secretCodeIndex];
        if (pressed & nextKey) {
            secretCodeIndex++;
            if (secretCode[secretCodeIndex] == 0) {
                secretCodeIndex = 0;
                SndAudioMgr::sInstance->startSystemSe(SE_VOC_MA_THANK_YOU, 1);

                if (isReplayEnabled != 100) {
                    isReplayEnabled = 100;
                    MapReport("Replay Recording enabled!\n");
                    showSecretMessage(0x40, 0x41, 9);
                } else {
                    isReplayEnabled = 0;
                    MapReport("Replay Recording disabled!\n");
                    showSecretMessage(0x50, 0x51, 2);
                }
            }
            return;
        } else {
            secretCodeIndex = 0;
        }
    }

    if (pressed & WPAD_BUTTON_MINUS) {
        minusCount++;
        if (minusCount >= 16) {
            minusCount = 0;

            enableDebugMode = !enableDebugMode;
            if (enableDebugMode) {
                SndAudioMgr::sInstance->startSystemSe(SE_VOC_MA_GET_PRIZE, 1);
                showSecretMessage(0x60, 0x61, 9, 0x6A, 9);
            } else {
                showSecretMessage(0x80, 0x81, 6);
            }
        }
    } else if (pressed & WPAD_BUTTON_1) {
        SndAudioMgr::sInstance->startSystemSe(SE_SYS_BACK, 1);
        mWillExit = true;
        mStateMgr.changeState(StateID_HideSectionWait);
    } else if ((pressed & WPAD_BUTTON_UP) && canScrollLeft()) {
        mCurrentWorld = mWorldIndices[--mCurrentWorldIndex];
        mWillExit = false;
        mStateMgr.changeState(StateID_HideSectionWait);
    } else if ((pressed & WPAD_BUTTON_DOWN) && canScrollRight()) {
        mCurrentWorld = mWorldIndices[++mCurrentWorldIndex];
        mWillExit = false;
        mStateMgr.changeState(StateID_HideSectionWait);
    }
}
void dWMStarCoin_c::finalizeState_Wait() { }



void dWMStarCoin_c::initializeState_HideSectionWait() {
    mLayout.AnimeStartSetup(ANIM_HIDE_SECTION, false);
    if (mWillExit) {
        hideLeftArrow();
        hideRightArrow();
    } else {
        SndAudioMgr::sInstance->startSystemSe(SE_SYS_PAGE, 1);
        setLeftArrowVisible(canScrollLeft());
        setRightArrowVisible(canScrollRight());
    }
}
void dWMStarCoin_c::executeState_HideSectionWait() {
    if (!mLayout.isAnime(ANIM_HIDE_SECTION)) {
        if (mWillExit)
            mStateMgr.changeState(StateID_HideWait);
        else
            mStateMgr.changeState(StateID_ShowSectionWait);
    }
}
void dWMStarCoin_c::finalizeState_HideSectionWait() { }



void dWMStarCoin_c::initializeState_HideWait() {
    mLayout.AnimeStartSetup(ANIM_SHOW_ALL, true);
    mLayout.GetAnimGroup()[ANIM_SHOW_ALL].mFrameCtrl.mFlags = 3; // NO_LOOP | REVERSE
}
void dWMStarCoin_c::executeState_HideWait() {
    if (!mLayout.isAnime(ANIM_SHOW_ALL))
        mStateMgr.changeState(StateID_Hidden);
}
void dWMStarCoin_c::finalizeState_HideWait() {
    mIsVisible = false;
}
#endif