#pragma once
#include <game/bases/d_collection_coin_date.hpp>

/// @unofficial
class dCollectionCoinBase_c : public dBase_c {
public:
    enum N_PANE_e {
        N_coinDateAll_00,
        N_coinBarPos_00, N_coinBarPos_01, N_coinBarPos_02, N_coinBarPos_03,
        N_coinBarPos_04, N_coinBarPos_05, N_coinBarPos_06, N_coinBarPos_07,
        N_coinBarPos_08, N_coinBarPos_09, N_coinBarPos_10, N_coinBarPos_11,
        N_COUNT
    };

    dCollectionCoinDate_c *mpCoinDates[12];
    LytBase_c mLayout;
    nw4r::lyt::Pane *mpRootPane;

    nw4r::lyt::Pane *mpNullPanes[N_COUNT];

    bool mHasLayoutLoaded;
    bool mIsVisible;
    u8 mPad[2];
};