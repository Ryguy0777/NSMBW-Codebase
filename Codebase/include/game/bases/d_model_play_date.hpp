#pragma once

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

/// @unofficial
class dModelPlayDate_c : public dBase_c {
public:
    enum ACTION_e {
        ACTION_ON_BUTTON,
        ACTION_IDLE_BUTTON,
        ACTION_HIT_BUTTON,
        ACTION_COUNT_COIN,
        ACTION_PURCHASE_MOVIE,
        ACTION_OFF_BUTTON,
        ACTION_ON_BUTTON_2,
        ACTION_OFF_BUTTON_2,
        ACTION_NONE
    };

    enum STATUS_e {
        STATUS_DISABLED, // Hides the button
        STATUS_LOCKED,
        STATUS_NOT_PURCHASED,
        STATUS_AVAILABLE
    };

    enum MOVIE_TYPE_e {
        SUPER_SKILLS,
        ENDLESS_LIVES,
        STAR_COIN,
        SECRET_GOAL
    };

    enum T_PANE_e {
        T_modelTitle_00,
        T_needCoin_01,
        T_worldNum_00,
        T_worldCourse_00,
        T_pictureFont_00,
        T_COUNT
    };

    enum P_PANE_e {
        P_model1_00, P_model1S_00, P_modelBefore1,
        P_starCoin_00,
        P_wave_00,
        P_COUNT
    };

    enum N_PANE_e {
        N_Coin_00,
        N_COUNT
    };

    enum W_PANE_e {
        W_model1_00,
        W_COUNT
    };

    sFStateMgr_c<dModelPlayDate_c, sStateMethodUsr_FI_c> mStateMgr;
    LytBase_c mLayout;
    nw4r::lyt::Pane *mpRootPane;

    LytTextBox_c *mpTextBoxes[T_COUNT];
    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
    nw4r::lyt::Pane *mpNullPanes[N_COUNT];
    nw4r::lyt::Window *mpWindowPanes[W_COUNT];

    bool mHasLayoutLoaded;
    bool mIsVisible;
    bool mWillUpdate;
    bool mIsAnimating;

    ACTION_e mPreviousAction;
    ACTION_e mCurrentAction;

    int mMovieCost;
    int mWorldNum;
    int mCourseNum;

    STATUS_e mCurrentStatus;
    STATUS_e mPreviousStatus;

    MOVIE_TYPE_e mMovieType;
    int mButtonIdx;
};