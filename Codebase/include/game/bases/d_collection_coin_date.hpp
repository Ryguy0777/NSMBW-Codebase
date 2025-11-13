#pragma once

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

/// @unofficial
class dCollectionCoinDate_c : public dBase_c {
public:
    enum N_PANE_e {
        N_coinBarDate_00,
        N_COUNT
    };

    enum P_PANE_e {
        P_coinBase1_00, P_coinBase2_00, P_coinBase3_00,
        P_coin_1_00, P_coin_2_00, P_coin_3_00,
        P_topFileter,
        P_COUNT
    };

    enum T_PANE_e {
        T_worldNum_00,
        T_dash_00,
        T_pictureFont_00, T_corseNum_00,
        T_COUNT
    };

    enum W_PANE_e {
        W_fileter_00,
        W_COUNT
    };

    LytBase_c mLayout;
    sFStateMgr_c<dCollectionCoinDate_c, sStateMethodUsr_FI_c> mStateMgr;
    nw4r::lyt::Pane *mpRootPane;

    nw4r::lyt::Pane *mpNullPanes[N_COUNT];
    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
    LytTextBox_c *mpTextBoxes[T_COUNT];
    nw4r::lyt::Window *mpWindowPanes[W_COUNT];

    int mWorldNumber;
    int mLevelNumber;
    int mCourseFlags;

    bool mHasLayoutLoaded;
    bool mIsVisible;
    bool mInactive;
    bool mMustUpdateInfo;
};