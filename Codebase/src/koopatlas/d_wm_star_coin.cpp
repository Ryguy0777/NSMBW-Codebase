#include <kamek.h>
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_wm_star_coin.hpp>
#include <new/constants/message_list.h>
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
        mLayout.mClipSettings.mEnabled = true;
        mLayout.mClipSettings.mPos.x = 0;
        mLayout.mClipSettings.mPos.y = 52;
        mLayout.mClipSettings.mSize.x = 640;
        mLayout.mClipSettings.mSize.y = 352;
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
        mLayout.AnimeStartSetup(SHOW_LEFT_ARROW, false);
        mpPicturePanes[DPadLeft]->SetVisible(true);
    }
}

void dWMStarCoin_c::showRightArrow() {
    if (!mRightArrowActive) {
        mRightArrowActive = true;
        mLayout.AnimeStartSetup(SHOW_RIGHT_ARROW, false);
        mpPicturePanes[DPadRight]->SetVisible(true);
    }
}

void dWMStarCoin_c::hideLeftArrow() {
    if (mLeftArrowActive) {
        mLeftArrowActive = false;
        mLayout.AnimeStartSetup(HIDE_LEFT_ARROW, false);
        mpPicturePanes[DPadLeft]->SetVisible(false);
    }
}

void dWMStarCoin_c::hideRightArrow() {
    if (mRightArrowActive) {
        mRightArrowActive = false;
        mLayout.AnimeStartSetup(HIDE_RIGHT_ARROW, false);
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
    /*int unspentCoins = getUnspentStarCoinCount();
    int coins = getStarCoinCount();

    WriteNumberToTextBox(&unspentCoins, UnspentCoinCount, false);
    WriteNumberToTextBox(&coins, TotalCoinCount, false);*/

    mCurrentWorld = -1;
    mCurrentWorldIndex = -1;
    mOpenWorldCount = 0;

    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    //int wantedSection = save->newerWorldID;
    int wantedSection = 0;

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

    // if we didn't find the wanted one, use the first one available
    if (mCurrentWorldIndex == -1) {
        mCurrentWorldIndex = 0;
        mCurrentWorld = mWorldIndices[0];
    }
}

// TODO: Load world names from BMG
void dWMStarCoin_c::loadSectionInfo() {
    dLevelInfo_c::entry_s *visibleLevels[COLUMN_COUNT][ROW_COUNT];

    // reset everything... everything
    for (int i = 0; i < COLUMN_COUNT; i++) {
        for (int j = 0; j < SHINE_COUNT; j++)
            mpShines[i][j]->SetVisible(false);

        for (int j = 0; j < ROW_COUNT; j++) {
            visibleLevels[i][j] = 0;

            mpLevelNames[i][j]->SetVisible(false);

            for (int k = 0; k < 3; k++) {
                mpCoinOutlines[i][j][k]->SetVisible(false);
                mpCoinOutlines[i][j][k]->SetVisible(false);
            }
        }
    }

    // get everything we'll need
    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    dLevelInfo_c *linfo = &dLevelInfo_c::m_instance;

    dLevelInfo_c::entry_s *names[COLUMN_COUNT];
    for (int i = 0; i < COLUMN_COUNT; i++)
        names[i] = linfo->getEntryFromDispID(mCurrentWorld, 100+i);

    bool useSubworlds = (COLUMN_COUNT > 1) && names[1];

    int currentPosition[COLUMN_COUNT];
    int currentColumn = 0; // only incremented in single-subworld mode

    for (int i = 0; i < COLUMN_COUNT; i++)
        currentPosition[i] = 0;

    dLevelInfo_c::section_s *section = linfo->getSection(mCurrentWorld);

    int earnedCoins = 0, earnableCoins = 0;
    // earnedCoins is calculated later

    for (int i = 0; i < section->mLevelCount; i++) {
        dLevelInfo_c::entry_s *level = &section->mLevels[i];

        // only pay attention to real levels
        if (!(level->mFlag & dLevelInfo_c::FLAG_VALID_LEVEL))
            continue;

        earnableCoins += 3;

        // is this level unlocked?
        u32 conds = save->getCourseDataFlag(level->mWorldSlot, level->mLevelSlot);

        // TODO: implement this
        //if (!(conds & COND_UNLOCKED))
        //	continue;

        // well, let's give it a slot
        if (useSubworlds) {
            currentColumn = (level->mFlag & dLevelInfo_c::FLAG_SECOND_HALF) ? 1 : 0;
        } else {
            if (currentPosition[currentColumn] >= ROW_COUNT)
                currentColumn++;
        }

        visibleLevels[currentColumn][currentPosition[currentColumn]++] = level;
    }

    // if the first column is empty, then move the second one over
    if (currentPosition[0] == 0 && useSubworlds) {
        for (int i = 0; i < currentPosition[1]; i++) {
            visibleLevels[0][i] = visibleLevels[1][i];
            visibleLevels[1][i] = 0;
        }

        names[0] = names[1];
        names[1] = 0;
    }

    // if the second column is empty, remove its name
    if (currentPosition[1] == 0 && useSubworlds)
        names[1] = 0;

    // work out the names
    //WriteAsciiToTextBox(LeftTitle, linfo->getLevelName(names[0]));
    //if (names[1])
    //	WriteAsciiToTextBox(RightTitle, linfo->getLevelName(names[1]));
    mpTextBoxes[RightTitle]->SetVisible(names[1] != 0);

    // load all level info
    for (int col = 0; col < COLUMN_COUNT; col++) {
        for (int row = 0; row < ROW_COUNT; row++) {
            dLevelInfo_c::entry_s *level = visibleLevels[col][row];
            if (!level)
                continue;

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
                    earnedCoins++;
                }
            }

            mpLevelNames[col][row]->SetVisible(true);
            //WriteAsciiToTextBox(LevelName[col][row], linfo->getLevelName(level));
        }
    }

    // set up coin things
    //WriteNumberToTextBox(&earnedCoins, EarnedCoinCount, false);
    //WriteNumberToTextBox(&earnableCoins, EarnedCoinMax, false);
}


void dWMStarCoin_c::initializeState_Hidden() { }
void dWMStarCoin_c::executeState_Hidden() { }
void dWMStarCoin_c::finalizeState_Hidden() { }

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

void dWMStarCoin_c::initializeState_ShowWait() {
    mIsVisible = true;
    loadInfo();
    mLayout.AnimeStartSetup(SHOW_ALL, false);
    mLayout.ReverseAnimeStartSetup(SHOW_SECTION, false);
    mLayout.ReverseAnimeStartSetup(SHOW_LEFT_ARROW, false);
    mLayout.ReverseAnimeStartSetup(SHOW_RIGHT_ARROW, false);

    secretCodeIndex = 0;
    minusCount = 0;
}
void dWMStarCoin_c::executeState_ShowWait() {
    if (!mLayout.isAnime(SHOW_ALL))
        mStateMgr.changeState(StateID_ShowSectionWait);
}
void dWMStarCoin_c::finalizeState_ShowWait() { }

void dWMStarCoin_c::initializeState_ShowSectionWait() {
    //loadSectionInfo();
    mLayout.AnimeStartSetup(SHOW_SECTION, false);

    if (canScrollLeft())
        showLeftArrow();
    if (canScrollRight())
        showRightArrow();
}
void dWMStarCoin_c::executeState_ShowSectionWait() {
    if (!mLayout.isAnime(SHOW_SECTION))
        mStateMgr.changeState(StateID_Wait);
}
void dWMStarCoin_c::finalizeState_ShowSectionWait() { }

void dWMStarCoin_c::showSecretMessage(const wchar_t *title, const wchar_t **body, int lineCount, const wchar_t **body2, int lineCount2) {
    return;
    /*LeftTitle->SetVisible(true);
    LeftTitle->SetString(title);
    RightTitle->SetVisible(false);

    for (int c = 0; c < COLUMN_COUNT; c++) {
        for (int i = 0; i < SHINE_COUNT; i++)
            Shine[c][i]->SetVisible(false);
        for (int r = 0; r < ROW_COUNT; r++) {
            LevelName[c][r]->SetVisible(false);
            for (int i = 0; i < 3; i++) {
                CoinOutline[c][r][i]->SetVisible(false);
                Coin[c][r][i]->SetVisible(false);
            }
        }
    }

    for (int i = 0; i < lineCount; i++) {
        LevelName[0][i]->SetVisible(true);
        LevelName[0][i]->SetString(body[i]);
    }

    if (body2) {
        for (int i = 0; i < lineCount2; i++) {
            LevelName[1][i]->SetVisible(true);
            LevelName[1][i]->SetString(body2[i]);
        }
    }*/
}

void dWMStarCoin_c::initializeState_Wait() { }
void dWMStarCoin_c::executeState_Wait() {
    int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;

    // A and Plus
    /*if ((dGameKey_c::m_instance->mRemocon[0]->mDownButtons == 0x810) && (pressed & 0x810)) {

        const int lineCountOn = 9, lineCountOff = 2;
        static const wchar_t *linesOn[lineCountOn] = {
            L"You've activated Hard Mode!",
            L" ",
            L"In Hard Mode, Mario will die",
            L"any time he takes damage, and",
            L"the timer will be more strict.",
            L" ",
            L"So treasure your Yoshi and",
            L"hold on to your hat, 'cause",
            L"you're in for a wild ride!",
        };
        static const wchar_t *linesOff[lineCountOff] = {
            L"Hard Mode has been",
            L"turned off.",
        };

        if (!enableHardMode) {
            enableHardMode = true;
            OSReport("Hard Mode enabled!\n");
            SndAudioMgr::sInstance->startSystemSe(SE_VOC_MA_CS_COURSE_IN_HARD, 1);
            showSecretMessage(L"Hard Mode", linesOn, lineCountOn);
        } else {
            enableHardMode = false;
            OSReport("Hard Mode disabled!\n");
            showSecretMessage(L"Classic Mario", linesOff, lineCountOff);
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
                //enableDebugMode = !enableDebugMode;
                //OSReport("Debug mode toggled!\n");
                const int lineCountOn = 9, lineCountOff = 2;
                static const wchar_t *linesOn[lineCountOn] = {
                    L"The experimental Replay",
                    L"Recording feature has",
                    L"been enabled. Enjoy!",
                    L"You'll find your Replays",
                    L"on your SD or USB, depending",
                    L"on where Newer's files are.",
                    L"It might not work, so",
                    L"save your game before you",
                    L"play a level!",
                };
                static const wchar_t *linesOff[lineCountOff] = {
                    L"Replay Recording",
                    L"turned off.",
                };

                if (isReplayEnabled != 100) {
                    isReplayEnabled = 100;
                    OSReport("Replay Recording enabled!\n");
                    showSecretMessage(L"Nice!", linesOn, lineCountOn);
                } else {
                    isReplayEnabled = 0;
                    OSReport("Replay Recording disabled!\n");
                    showSecretMessage(L"Nice!", linesOff, lineCountOff);
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

                const int msgCount = 9;
                static const wchar_t *msg[msgCount] = {
                    L"You've found the Totally",
                    L"Secret Collision Debug Mode.",
                    L"We used this to make the",
                    L"hitboxes on our custom sprites",
                    L"and bosses suck less. Awesome,",
                    L"right?!",
                    L"Actually, I did it just to waste",
                    L"some time, but it ended up",
                    L"being pretty useful!",
                };
                const int msgCount2 = 9;
                static const wchar_t *msg2[msgCount2] = {
                    L"And yes, I know it doesn't show",
                    L"a couple of things properly",
                    L"like round objects and rolling",
                    L"hills and so on.",
                    L"Can't have it all, can you?",
                    L"Wonder if Nintendo had",
                    L"something like this...",
                    L"",
                    L"    Treeki, 9th February 2013",
                };
                showSecretMessage(L"Groovy!", msg, msgCount, msg2, msgCount2);
            } else {
                const int msgCount = 6;
                static const wchar_t *msg[msgCount] = {
                    L"You've turned off the Totally",
                    L"Secret Collision Debug Mode.",
                    L"",
                    L"... and no, I'm not going to write",
                    L"another ridiculously long",
                    L"message to go here. Sorry!",
                };
                static const wchar_t *hiddenMsg[] = {
                    L"If you found these messages by",
                    L"looking through strings in the DLCode",
                    L"file, then... that's kind of cheating.",
                    L"Though I can't say I wouldn't do the",
                    L"same!",
                    L"You won't actually see this in game",
                    L"btw :p So why am I bothering with linebreaks anyway? I dunno. Oh well.",
                    L"Also, don't put this message on TCRF. Or do! Whatever. :(",
                };
                showSecretMessage(L"Groovy!", msg, msgCount, hiddenMsg, 0);
            }
        }
    } else*/ if (pressed & WPAD_BUTTON_1) {
        SndAudioMgr::sInstance->startSystemSe(SE_SYS_DIALOGUE_OUT_AUTO, 1);
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
    mLayout.AnimeStartSetup(HIDE_SECTION, false);
    if (mWillExit) {
        hideLeftArrow();
        hideRightArrow();
    } else {
        setLeftArrowVisible(canScrollLeft());
        setRightArrowVisible(canScrollRight());
    }
}
void dWMStarCoin_c::executeState_HideSectionWait() {
    if (!mLayout.isAnime(HIDE_SECTION)) {
        if (mWillExit)
            mStateMgr.changeState(StateID_HideWait);
        else
            mStateMgr.changeState(StateID_ShowSectionWait);
    }
}
void dWMStarCoin_c::finalizeState_HideSectionWait() { }

void dWMStarCoin_c::initializeState_HideWait() {
    mLayout.AnimeStartSetup(SHOW_ALL, true);
    mLayout.GetAnimGroup()[SHOW_ALL].mFrameCtrl.mFlags = 3; // NO_LOOP | REVERSE
}
void dWMStarCoin_c::executeState_HideWait() {
    if (!mLayout.isAnime(SHOW_ALL))
        mStateMgr.changeState(StateID_Hidden);
}
void dWMStarCoin_c::finalizeState_HideWait() {
    mIsVisible = false;
}
#endif