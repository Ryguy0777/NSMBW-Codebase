#pragma once

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/bases/d_stock_item_shadow.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>
#include <lib/egg/eggEffect.hpp>

/// @unofficial
class dStockItem_c : public dBase_c {
public:
    enum N_PANE_e {
        N_forUse_1PPos, N_forUse_2PPos, N_forUse_3PPos, N_forUse_4PPos,
        N_iconKinoko_00, N_iconFlower_00, N_iconPro_00, N_iconIce_00,
        N_iconPen_00, N_mameKinoko_00, N_iconStar_00,
        N_stockItem, N_stockItem_01, N_itemSelect_00,
        N_COUNT
    };

    enum P_PANE_e {
        P_iconKinoko_00, P_iconFlower_00, P_iconPro_00, P_iconIce_00,
        P_iconPen_00, P_mameKinoko_00, P_iconStar_00,
        P_buttonBase_00, P_buttonBase_01, P_buttonBase_02, P_buttonBase_06,
        P_buttonBase_04, P_buttonBase_05, P_buttonBase_03,
        P_iconBase_00, P_iconBase_01, P_iconBase_02, P_iconBase_03, P_iconBase_04,
        P_iconBase_05, P_iconBase_06, P_iconBase_07, P_iconBase_08, P_iconBase_09,
        P_COUNT
    };

    u32 m_70;
    LytBase_c mLayout;
    sFStateMgr_c<dStockItem_c, sStateMethodUsr_FI_c> mStateMgr;
    nw4r::lyt::Pane *mpRootPane;

    nw4r::lyt::Pane *mpNullPanes[N_COUNT];
    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];

    void *mpPlayers[4]; // TODO: da2DPlayer_c
    void *mpItems[7]; // TODO: daWmItem_c
    dStockItemShadow_c *mpShadow;

    int mMovementDirection;
    u32 m_318;
    u32 m_31C;
    u32 m_320;
    u32 m_324;
    int mItemUsed;

    EGG::Effect mPlayerEffects[4];
    EGG::Effect mButtonEffect;
    int mEffectIDs[4];

    int mCurrentSelection;
    int mPreviousSelection;
    int mMenuCloseTimer;
    int mSuccessionSelectDelay;
    int mPlayerCount;
    int mItemCounts[7];
    int mPaneIdx;
    int mAnimID;

    bool mPlayerActive[4];
    bool mHasLayoutLoaded;
    bool mIsVisible;
    bool mSuccessionSelectActive;
};