#pragma once

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>

/// @unofficial
class dSelectCursor_c : public dBase_c {
public:
    class Layout_c {
    public:
        enum P_PANE_e {
            P_cursor_00,
            P_cursor_01,
            P_cursor_02,
            P_cursor_03,
            P_COUNT
        };

        enum N_PANE_e {
            N_cursor_00,
            N_LU_00, N_RU_00,
            N_LD_00, N_RD_00,
            N_COUNT
        };

        LytBase_c mLayout;
        nw4r::lyt::Pane *mpRootPane;

        nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
        nw4r::lyt::Pane *mpNullPanes[N_COUNT];

        bool mIsActive;

        char mTargetPaneName[17];
        u8 mPad[0x92];
    };

    d2d::ResAccMultLoader_c mLytLoader;
    Layout_c mCursors[5];

    bool mHasLayoutLoaded;
    u8 mPad[3];
};