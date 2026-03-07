#pragma once

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

/// @unofficial
class dCourseSelectMenu_c : public dBase_c {
public:
    enum P_PANE_e {
        P_SBBase_00, P_SBBase_01,
        P_SBBase_02, P_SBBase_03,
        P_back, P_backWhite,
        P_COUNT
    };

    enum T_PANE_e {
        T_corseSelectS03, T_corseSelect_03,
        T_COUNT
    };

    LytBase_c mLayout;
    sFStateMgr_c<dCourseSelectMenu_c, sStateMethodUsr_FI_c> mStateMgr;
    nw4r::lyt::Pane *mpRootPane;

    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
    LytTextBox_c *mpTextBoxes[T_COUNT];

    int mCurrentSelection;
    int mPreviousSelection;

    bool mHasLayoutLoaded;
    bool mIsVisible;
    bool mChoiceMade;
    bool mSkipNavigation;
    bool m_274;
    u8 mPad[3];
};