#pragma once

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

/// @unofficial
class dContinue_c : public dBase_c {
public:
    enum P_PANE_e {
        P_bg_PContinue,
        P_COUNT
    };

    enum N_PANE_e {
        N_4P_Pos_00, N_4P_Pos_01, N_4P_Pos_02, N_4P_Pos_03,
        N_3P_Pos_00, N_3P_Pos_01, N_3P_Pos_02,
        N_2P_Pos_00, N_2P_Pos_01,
        N_1P_Pos_00,
        N_dateFile_01P, N_dateFile_02P, N_dateFile_03P, N_dateFile_04P,
        N_COUNT
    };

    enum T_PANE_e {
        T_conTime_00, T_conTime_01, T_conTime_02, T_conTime_03,
        T_playerZanki_04, T_playerZanki_05, T_playerZanki_06, T_playerZanki_07,
        T_COUNT
    };

    LytBase_c mLayout;
    sFStateMgr_c<dContinue_c, sStateMethodUsr_FI_c> mStateMgr;
    nw4r::lyt::Pane *mpRootPane;

    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
    nw4r::lyt::Pane *mpNullPanes[N_COUNT];
    LytTextBox_c *mpTextBoxes[T_COUNT];

    int mPlayerCount;
    int mContinues[4];
    int mLives[4];

    int mAddedLives;
    int mCountdown;
    int mExitCountdown;

    bool mHasLayoutLoaded;
    bool mIsVisible;
    bool mWillOpen;
    bool mWillClose;

    bool mUsingContinue[4];

    int m_2dc;

    bool mIsGameOver;
    u8 mPad[3];
};