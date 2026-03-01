#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/koopatlas_config.h>

#ifdef KP_PATH_MNG_REPORT
#define PathMngReport OSReport
#else
#define PathMngReport(...)
#endif

#ifdef KP_UNLOCK_DEBUG
#define UnlockCmdReport OSReport
#else
#define UnlockCmdReport(...)
#endif

#include <game/bases/d_mj2d_data.hpp>
#include <new/bases/koopatlas/d_kp_map_data.hpp>
#include <new/bases/d_level_info.hpp>
#include <nw4r/snd/snd_SoundHandle.h>

class dKpPathManager_c {
public:
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

    struct CompletionData_s {
        u8 mPrevLevelID[2]; // Last level entered
        bool mChkSavePrompt; // Checks if a save prompt should appear after a course. Only applies for the first clear

        bool mIsCmpCoins;
        bool mIsCmpExits;
        bool mIsCmpWorld;

        bool mIsCmpCoinsExcW9;
        bool mIsCmpAllCoins;
        bool mIsCmpAllExits;
        bool mIsCmpAll;
    };

    ~dKpPathManager_c();

    void create();
    void execute();

    bool isPathMgrActive();

    bool canUseExit(dKPPath_s *path) {
        OSReport("Checking usability of path %p\n", path);
        if (path != nullptr) {
            OSReport("Open status: %d\n", path->mIsOpen);
            return path->mIsOpen;
        }
        return false;
    }

    int getPressedDir(int buttons);
    void startMovementTo(dKPPath_s *path);
    void moveThroughPath(int pressedDir);

    void activatePoint();
    void unlockAllPaths(char type);

    void copyWorldDefToSave(const dKPWorldDef_s *world);
private:
    void unlockPaths();
    void finalisePathUnlocks();
    bool evaluateUnlockCondition(u8 *&in, dMj2dGame_c *save, int stack);

    void visitNodeForCamCheck(dKPNode_s *node);
    void findCameraBoundsForUnlockedPaths();
    void addNodeToCameraBounds(dKPNode_s *node);

public:
    static void clearPathData();
    static void resetCompletionData();

    dKPLayer_s *mpPathLayer;
    dKPNode_s *mpCurrentNode;
    dKPPath_s *mpCurrentPath;
    dLevelInfo_c::entry_s *mEnteredLevel;
    nw4r::snd::SoundHandle mPenguinSlideHandle;

    float mTimer;
    float mMoveSpeed;

    int mCmpAnimDuration;
    int mCmpMsgDelay;
    int mCmpMsgType;

    int mCourseInDelay;
    int mInitialDelay;
    int mInitialPlyAnmDelay;
    int mPathFadeInDelay;
    int mPanBackDelay;

    int mNodeUnlockDuration;
    int mScaleDuration;
    
    int mPathUnlockAlpha; // -1 if not used

    int mNewOpenNodeNum;
    int mNewOpenPathNum;

private:
    int mCollectStarCoinNum;
    int mUnspentStarCoinNum;

    int mCamMinX;
    int mCamMinY;
    int mCamMaxX;
    int mCamMaxY;
    int mNodeStackLen;

public:
    bool mDoPlayPenguinSlide; // Special check for penguin slip sound

    bool mIsMoving;
    bool mIsJumpAnm;
    bool mIsSwimAnm;

    bool mReverseOnPath;
    bool mDisableInput;

    bool mIsScaleUp;
    bool mIsForceAngle;
    bool mIsPassThrough;

    bool mIsEnterNode;
    bool mIsCourseIn;
    bool mDoCourseFailAnm;
    bool mDoCourseClearAnm;

    bool mInitialLoadComplete;

    bool mDispSavePrompt;
    bool mIsCmpMsgActive;
    bool mDidAutoWalkCheck;
    bool mAutoWalkMode;

    bool mDidEndingSceneSave;
    bool mEndingSceneComplete;

    // TODO: Decide what to do with this. Enabled if Map Change destination node is NULL
    // But it never does anything from there.
    bool mustComplainToMapCreator;

    bool mIsCamPanToPaths;
    bool mIsCamPanFromPaths;

private:
    bool mIsCamBoundsValid;

public:
    static CompletionData_s s_cmpData;
    static u8 *sp_openPathData;
    static u8 *sp_openNodeData;
};

#endif

