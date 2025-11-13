#pragma once
#include <constants/game_constants.h>
#include <game/bases/d_collection_coin_base.hpp>
#include <game/bases/d_model_play_arrow.hpp>
#include <game/bases/d_2d.hpp>

/// @unofficial
class dCollectionCoin_c : public dBase_c {
public:
    struct CourseData_s {
        int mWorldNum;
        int mLevelNum;
        int mCondition; // Set to -1 if level has no coins, or if mLevelNum is 42
        u32 mPad;
    };

    enum N_PANE_e {
        N_coinbar_01, N_coinbar_00,
        N_Scissor_00,
        N_wCC_00,
        N_arrowPos,
        N_COUNT
    };

    enum T_PANE_e {
        T_world_00, T_world_01,
        T_COUNT
    };

    u32 m_70;
    dCollectionCoinBase_c *mpCoinBases[2];
    dCollectionCoinDate_c *mpCoinDates[24];
    dModelPlayArrow_c *mpArrow;

    LytBase_c mLayout;
    d2d::ResAccMultLoader_c mBaseLytRes;
    d2d::ResAccMultLoader_c mDateLytRes;
    sFStateMgr_c<dCollectionCoin_c, sStateMethodUsr_FI_c> mStateMgr;
    nw4r::lyt::Pane *mpRootPane;

    nw4r::lyt::Pane *mpNullPanes[N_COUNT];
    LytTextBox_c *mpTextBoxes[T_COUNT];

    CourseData_s mCourseDatas[WORLD_USED_COUNT][12];

    int mTextBoxToUpdate; // 1: T_world_00, 2: T_world_01
    int mArrowHit; // 1: Left, 2: Right

    int mCurrentWorld;
    int mOpenWorldCount;

    bool mHasLayoutLoaded;
    bool mIsVisible;

    u8 mScrollDirection; // 0: Previous page, 1: Next page

    bool mHasBaseResLoaded;
    bool mHasDateResLoaded;
    bool mIsWorldOpen[10];
    u8 mPad;
};