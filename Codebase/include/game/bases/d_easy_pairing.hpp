#pragma once

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

/// @unofficial
class dEasyPairing_c : public dBase_c {
public:
    enum P_PANE_e {
        P_remoS_03, P_remoS_00, P_remoS_01, P_remoS_02,
        P_lamp_00, P_lamp_01, P_lamp_02, P_lamp_03,
        P_light_00, P_light_01, P_light_02, P_light_03,
        P_COUNT
    };

    LytBase_c mLayout;
    sFStateMgr_c<dEasyPairing_c, sStateMethodUsr_FI_c> mStateMgr;
    nw4r::lyt::Pane *mpRootPane;

    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];

    bool mHasLayoutLoaded;
    bool mIsVisible;
    bool mWillStop;
    bool mWillStart;

    int mConnectStates[4];
    int m_28c[4];
    int m_29c[4];
};