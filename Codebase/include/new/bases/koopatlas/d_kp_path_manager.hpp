#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
//#define WM_UNLOCK_DEBUGGING
#ifdef WM_UNLOCK_DEBUGGING
#define UnlockCmdReport OSReport
#else
#define UnlockCmdReport(...)
#endif

#include <game/bases/d_mj2d_data.hpp>
#include <new/bases/koopatlas/d_kp_map_data.hpp>
#include <new/bases/d_level_info.hpp>
#include <nw4r/snd/snd_SoundHandle.h>

typedef struct { f32 frame, value, slope; } HermiteKey;

static u8 MaybeFinishingLevel[2] = {0xFF,0xFF};
static u8 LastLevelPlayed[2] = {0xFF,0xFF};

class dWMPathManager_c {
public:
    struct CompletionData_s {
        u8 mPrevLevelID[2]; // Last level entered (of any type)

        // TODO: remove this, just use mPrevLevelNum (rename it to that), then have a bool for controlling if a save-prompt check should be done
        u8 mSavePromptLevelID[2]; // Set for courses that will prompt a save afterwards. Only set if not cleared prior

        bool mIsCmpCoins;
        bool mIsCmpExits;
        bool mIsCmpWorld;

        bool mIsCmpCoinsExcW9;

        bool mIsCmpAllCoins;
        bool mIsCmpAllExits;
        bool mIsCmpAll;
    };

    enum CompletionType_e {
        CMP_MSG_NULL = 0,
        CMP_MSG_COINS,
        CMP_MSG_EXITS,
        CMP_MSG_WORLD,
        CMP_MSG_GLOBAL_COINS_EXC_W9,
        CMP_MSG_GLOBAL_COINS,
        CMP_MSG_GLOBAL_EXITS,
        CMP_MSG_EVERYTHING
    };

    void setup();
    ~dWMPathManager_c();
    void execute();

    bool canUseExit(dKPPath_s *path) {
        OSReport("Checking usability of path %p\n", path);
        if (path != nullptr) {
            OSReport("Open status: %d\n", path->mIsOpen);
            return path->mIsOpen;
        }
        return false;
    }

    void startMovementTo(dKPPath_s *path);
    void moveThroughPath(int pressedDir);
    void activatePoint();
    void unlockAllPaths(char type);

    void copyWorldDefToSave(const dKPWorldDef_s *world);

    dKPLayer_s *pathLayer;

    bool firstPathDone;
    bool isMoving;
    dKPNode_s *currentNode;

    HermiteKey keysX[2];
    HermiteKey keysY[3];
    float timer;
    bool isJumping;
    float moveSpeed;

    bool forcedRotation;

    int scaleAnimProgress;
    bool isScalingUp;

    nw4r::snd::SoundHandle penguinSlideSound;

    bool swimming;

    dKPPath_s *currentPath;
    bool reverseThroughPath; // direction we are going through the path

    bool mustComplainToMapCreator;

    int newlyAvailablePaths;
    int newlyAvailableNodes;

    bool panningCameraToPaths;
    bool panningCameraFromPaths;
    int unlockingAlpha; // -1 if not used
    int countdownToFadeIn;
    int waitAfterUnlock;
    int waitBeforePanBack;

    bool savingForEnding;
    bool waitingForEndingTransition;

private:
    void unlockPaths();
    void finalisePathUnlocks();
    bool evaluateUnlockCondition(u8 *&in, dMj2dGame_c *save, int stack);
    int cachedTotalStarCoinCount;
    int cachedUnspentStarCoinCount;

public:
    bool shouldRequestSave;
    bool isEnteringLevel;
    bool completionMessagePending;
    int dmGladDuration;
    int completionAnimDelay;
    int completionMessageType;
    int completionMessageWorldNum;

    bool calledEnteredNode;
    int levelStartWait;
    int waitAtStart;
    bool mustPlayAfterDeathAnim;
    bool mustPlayAfterWinAnim;
    int waitAfterInitialPlayerAnim;
    bool initialLoading;

    bool checkedForMoveAfterEndLevel;
    bool afterFortressMode;

    bool doingThings();

    dLevelInfo_c::entry_s *enteredLevel;

private:
    int camMinX, camMinY, camMaxX, camMaxY;
    int nodeStackLength;
    bool camBoundsValid;
    void visitNodeForCamCheck(dKPNode_s *node);
    void findCameraBoundsForUnlockedPaths();
    void addNodeToCameraBounds(dKPNode_s *node);

public:
    static CompletionData_s s_cmpData;
};

#endif

