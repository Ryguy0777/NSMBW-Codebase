#pragma once

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>

/// @unofficial
class dWorldSelectGuide_c : public dBase_c {
public:
    enum T_PANE_e {
        T_worldNum_00,
        T_COUNT
    };

    enum P_PANE_e {
        P_pochi_00, P_pochi_01,
        P_pochi_02, P_pochi_03,
        P_COUNT
    };

    enum N_PANE_e {
        N_wCCGuide_00,
        N_title,
        N_COUNT
    };

    LytBase_c mLayout;
    nw4r::lyt::Pane *mpRootPane;

    LytTextBox_c *mpTextBoxes[T_COUNT];
    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
    nw4r::lyt::Pane *mpNullPanes[N_COUNT];

    bool mHasLayoutLoaded;
    bool mIsVisible;
    bool mShowPressIcons;
    bool mShowUpPressIcon;

    int mCurrentWorld;
    int mNewWorld;

    float mTitleTranslateY;
};