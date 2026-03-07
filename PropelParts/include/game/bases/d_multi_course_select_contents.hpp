#pragma once

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

/// @unofficial
class dMultiCourseSelectContents_c : public dBase_c {
public:
    enum P_PANE_e {
        P_enpty,
        P_corse_00,
        P_corseFrame_00, P_clearFrame_00,
        P_allClearF_00, P_clearBase_00,
        P_coin_00,
        P_COUNT
    };

    enum T_PANE_e {
        T_corseNum_00, T_corsePic_00,
        T_W_corseNum_00, T_W_corseNum_01,
        T_W_corseNum_02, T_W_corsePic_00,
        T_clear_00, T_allClear_00,
        T_COUNT
    };

    enum N_PANE_e {
        N_courseInfo_00,
        N_clear_00,
        N_COUNT
    };

    enum ACTION_e {
        ACTION_NONE,
        ACTION_ON,
        ACTION_HIT,
        ACTION_OFF,
        ACTION_PAUSE = 5,
        ACTION_CLEARED
    };

    enum CURSOR_STATE_e {
        STATE_HIDDEN,
        STATE_SHOW,
        STATE_HIDE
    };

    LytBase_c mLayout;
    sFStateMgr_c<dMultiCourseSelectContents_c, sStateMethodUsr_FI_c> mStateMgr;
    nw4r::lyt::Pane *mpRootPane;

    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
    LytTextBox_c *mpTextBoxes[T_COUNT];
    nw4r::lyt::Pane *mpNullPanes[N_COUNT];

    ACTION_e mAction;

    bool mHasLayoutLoaded;
    bool mIsVisible;
    bool mDoCursorUpdate;
    bool mIsAnimating;
    bool mIsSelected;
    u8 mPad[3];

    int mButtonIndex;
    int mCurrentPage;
    int mPreviousPage;

    CURSOR_STATE_e mCursorState;
    int mClearAnimeTimer;

    int mWorldNumber;
    int mLevelNumber;
};