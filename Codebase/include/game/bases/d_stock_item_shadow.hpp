#pragma once

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>
#include <lib/egg/eggEffect.hpp>

/// @unofficial
class dStockItemShadow_c : public dBase_c {
public:
    enum T_PANE_e {
        T_itemRest_00, T_itemRest_01, T_itemRest_02, T_itemRest_06, T_itemRest_04, T_itemRest_05, T_itemRest_03,
        T_itemRest_07, T_itemRest_08, T_itemRest_09, T_itemRest_10, T_itemRest_11, T_itemRest_12, T_itemRest_13,
        T_COUNT
    };

    enum P_PANE_e {
        P_buttonBaseS_00, P_buttonBaseS_01, P_buttonBaseS_02,P_buttonBaseS_03,
        P_buttonBaseS_04, P_buttonBaseS_05, P_buttonBaseS_06,
        P_COUNT
    };

    LytBase_c mLayout;
    nw4r::lyt::Pane *mpRootPane;

    LytTextBox_c *mpTextBoxes[T_COUNT];
    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];

    bool mHasLayoutLoaded;
    bool mIsVisible;
    bool mMustUpdate;
    u8 mPad;
    int mItemCounts[7];
};