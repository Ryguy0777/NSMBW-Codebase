#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/koopatlas_config.h>

#include <new/bases/koopatlas/d_s_koopatlas.hpp>

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

class dKPStarCoinMenu_c : public dBase_c {
public:
    enum P_PANE_e {
        DPadLeft, DPadRight,
        P_COUNT
    };

    enum T_PANE_e {
        LeftTitle, RightTitle,
        TotalCoinCount, UnspentCoinCount,
        EarnedCoinCount, EarnedCoinMax,
        BtnBackText,
        T_COUNT
    };

    enum ANIMS_e {
        ANIM_SHOW_ALL = 0,
        ANIM_SHOW_SECTION,
        ANIM_HIDE_SECTION,
        ANIM_SHOW_LEFT_ARROW,
        ANIM_HIDE_LEFT_ARROW,
        ANIM_SHOW_RIGHT_ARROW,
        ANIM_HIDE_RIGHT_ARROW
    };

    enum _Constants {
        ROW_COUNT = 9,
        COLUMN_COUNT = 2,
        SHINE_COUNT = 5,
        MAX_SECTION_COUNT = 16,
    };

    dKPStarCoinMenu_c();

    int create();
    int doDelete();
    int execute();
    int draw();

    void dispMenu();

    bool canScrollLeft() const;
    bool canScrollRight() const;

    void leftArrowDisp(bool value);
    void rightArrowDisp(bool value);

    void loadInfo();
    void loadSectionInfo();

    void displayMessage(int titleMsg, int bodyMsgStart, int bodyMsgCount, int bodyMsgStart2 = 0, int bodyMsgCount2 = 0);

    STATE_FUNC_DECLARE(dKPStarCoinMenu_c, Hidden);
    STATE_FUNC_DECLARE(dKPStarCoinMenu_c, ShowWait);
    STATE_FUNC_DECLARE(dKPStarCoinMenu_c, ShowSectionWait);
    STATE_FUNC_DECLARE(dKPStarCoinMenu_c, Wait);
    STATE_FUNC_DECLARE(dKPStarCoinMenu_c, HideSectionWait);
    STATE_FUNC_DECLARE(dKPStarCoinMenu_c, HideWait);

    LytBase_c mLayout;
    sFStateMgr_c<dKPStarCoinMenu_c, sStateMethodUsr_FI_c> mStateMgr;

    nw4r::lyt::Picture *mpShines[COLUMN_COUNT][SHINE_COUNT];
    nw4r::lyt::Picture *mpCoinOutlines[COLUMN_COUNT][ROW_COUNT][3];
    nw4r::lyt::Picture *mpCoins[COLUMN_COUNT][ROW_COUNT][3];
    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
    LytTextBox_c *mpLevelNames[COLUMN_COUNT][ROW_COUNT];
    LytTextBox_c *mpTextBoxes[T_COUNT];

    int mCurrentWorld;
    int mCurrentWorldIndex;
    int mWorldIndices[MAX_SECTION_COUNT];
    int mOpenWorldCount;

    bool mHasLayoutLoaded;
    bool mIsVisible;
    bool mLeftArrowActive;
    bool mRightArrowActive;
    bool mWillExit;

    static dKPStarCoinMenu_c *m_instance;
};
#endif