#pragma once

#include <game/bases/d_2d.hpp>
#include <game/bases/d_multi_course_select_contents.hpp>

/// @unofficial
class dMultiCourseSelect_c : public dBase_c {
public:
    struct LevelData_s {
        int mWorldNum;
        int mLevelNum;
        u16 m_08;
        u8 mPad[2];
        int m_0C;
    };

    enum P_PANE_e {
        P_Bg_00,
        P_arrowCL_00, P_arrowCR_00,
        P_title_00, P_title_00_vs,
        P_COUNT
    };

    enum N_PANE_e {
        N_Pbase_00, N_Pbase_01, N_Pbase_03, N_Pbase_02,
        N_1Player_Pos_00,
        N_2PlayerPos_00, N_2PlayerPos_01,
        N_3Player_Pos_00, N_3Player_Pos_01, N_3Player_Pos_02,
        N_Bg_00, N_Bg_00_vs,
        N_corse1_Pos_00, N_corse2_Pos_00, N_corse3_Pos_00, N_corse4_Pos_00, N_corse5_Pos_00,
        N_corse6_Pos_00, N_corse7_Pos_00, N_corse8_Pos_00, N_corse9_Pos_00, N_corse10_Pos_00,
        N_corse1_Pos_01, N_corse2_Pos_01, N_corse3_Pos_01, N_corse4_Pos_01, N_corse5_Pos_01,
        N_corse6_Pos_01, N_corse7_Pos_01, N_corse8_Pos_01, N_corse9_Pos_01, N_corse10_Pos_01,
        N_title_00,
        N_scissor_00,
        N_cursorSize_00,
        N_COUNT
    };

    enum T_PANE_e {
        T_titleMulti_00, T_titleMulti_01,
        T_rankStar_00, T_rankStar_01,
        T_rankStar_02, T_rankStar_03,
        T_guideViewC_00,
        T_COUNT
    };

    enum W_PANE_e {
        W_title_00,
        W_Base_00, W_Base_00_vs,
        W_Base_01, W_Base_01_vs,
        W_COUNT
    };

    LytBase_c mLayout;
    d2d::ResAccMultLoader_c mContentsLytRes;
    dMultiCourseSelectContents_c *mpContents[20];
    sFStateMgr_c<dMultiCourseSelect_c, sStateMethodUsr_FI_c> mStateMgr;
    nw4r::lyt::Pane *mpRootPane;

    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
    nw4r::lyt::Pane *mpNullPanes[N_COUNT];
    LytTextBox_c *mpTextBoxes[T_COUNT];
    nw4r::lyt::Window *mpWindowPanes[W_COUNT];

    int mScrollDirection;

    LevelData_s mFavoritedCourses[10];

    float mPBasePositions[4];
    float mArrowLoopAnimeFrame;

    int mCurrentSelection;
    int mPreviousSelection;
    int mSuccessionSelectDelay;
    int mCurrentPage; // 0 = recommended, 10 = favorites

    int mOpenWorldCount;
    u32 mKeyBitmask;

    bool mHasLayoutLoaded;
    bool mCanWindowOpen;
    bool mSuccessionSelectActive;

    // Used to control functionality in dScMulti_c
    bool mWillBeginCourse;
    bool mIsYesNoActive;
    bool mDoExitScene;

    bool mHasContentsResLoaded;
    bool mEnableLeftArrow;
    bool mEnableRightArrow;
    bool mIsWorldOpen[11];
    bool selectingWorld;
    u8 mPad[3];

    void loadTitleFromLI(int tb, int type); // New
    void colorTitleText(int tb, int type);
};