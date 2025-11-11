#pragma once

#include <game/bases/d_a_2d_player.hpp>
#include <game/bases/d_base.hpp>
#include <game/bases/d_character_change_select_base.hpp>
#include <game/bases/d_character_change_select_contents.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

/// @unofficial
class dNumberOfPeopleChange_c : public dBase_c {
public:
    enum N_PANE_e {
        N_pos1P_00, N_pos2P_00, N_pos3P_00, N_pos4P_00,
        N_pos1P_01, N_pos2P_01, N_pos3P_01,
        N_pos1P_02, N_pos2P_02,
        N_pos2P_01_,
        N_COUNT
    };

    enum T_PANE_e {
        T_titleNinzuMenu, T_titleNinzu_00,
        T_guide_01, T_guideS_02,
        T_COUNT
    };

    enum P_PANE_e {
        P_yesBase_00, P_noBase_00,
        P_bgShadowST_00, P_bgST_00,
        P_stripeMLT_00,
        P_remo1P_Light, P_remo2P_Light,
        P_remo3P_Light, P_remo4P_Light,
        P_base_00, P_base_01, P_base01_Menu43,
        P_COUNT
    };

    enum W_PANE_e { // Game loads the same pane twice
        W_button_00,
        W_button_00_,
        W_COUNT
    };

    u32 m_70;
    dCharacterChangeSelectBase_c *mpSelectBases[4];
    dCharacterChangeSelectContents_c *mpSelectContents[4];
    void *mpSelectArrows[4];
    void *mpIndicators[4];

    d2d::ResAccMultLoader_c mSelectBaseRes;
    d2d::ResAccMultLoader_c mSelectContentsRes;
    d2d::ResAccMultLoader_c mSelectArrowRes;
    d2d::ResAccMultLoader_c mIndicatorRes;

    LytBase_c mLayout;
    sFStateMgr_c<dNumberOfPeopleChange_c, sStateMethodUsr_FI_c> mStateMgr;
    nw4r::lyt::Pane *mpRootPane;

    nw4r::lyt::Pane *mpNullPanes[N_COUNT];
    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
    LytTextBox_c *mpTextBoxes[T_COUNT];
    nw4r::lyt::Window *mpWindowPanes[W_COUNT];

    da2DPlayer_c *mpPlayers[4];

    u8 mPad[0x20];

    bool mHasLayoutLoaded;
    u8 m_67d;
    bool mIsVisible;
    bool mIsEasyPairingActive;
    bool mAllowEasyPairing;
    bool mLoadedSelectBaseRes;
    bool mLoadedSelectContentsRes;
    bool mLoadedSelectArrowRes;
    u8 m_684;
    bool mLoadedIndicatorRes;

    u8 mPad2[6];
    u32 m_68c;

    int mCharIDs[4];
    int mPlayerCount;
    int mPaneIdx;

    u32 mPad3[7];

    int mPlayerCountForBMG;
    int mCurrentSelection;
    int mPreviousSelection;

    u8 mPad4[10];

    int mCharIDsCopy[4];
    int mCharIDsCopy2[4];

    u8 mPad5[10];
};