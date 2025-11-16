#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_s_koopatlas.hpp>

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

class dWMStarCoin_c : public dBase_c {
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
        SHOW_ALL = 0,
        SHOW_SECTION,
        HIDE_SECTION,
        SHOW_LEFT_ARROW,
        HIDE_LEFT_ARROW,
        SHOW_RIGHT_ARROW,
        HIDE_RIGHT_ARROW
    };

    enum _Constants {
        ROW_COUNT = 9,
        COLUMN_COUNT = 2,
        SHINE_COUNT = 5,
        MAX_SECTION_COUNT = 16,
    };

    dWMStarCoin_c();

    int create();
    int doDelete();
    int execute();
    int draw();

    void show();

    bool canScrollLeft() const;
    bool canScrollRight() const;
    void loadInfo();
    void loadSectionInfo();

    void showLeftArrow();
    void showRightArrow();
    void hideLeftArrow();
    void hideRightArrow();

    void setLeftArrowVisible(bool value);
    void setRightArrowVisible(bool value);

    void showSecretMessage(const wchar_t *title, const wchar_t **body, int lineCount, const wchar_t **body2 = 0, int lineCount2 = 0);

    LytBase_c mLayout;
    sFStateMgr_c<dWMStarCoin_c, sStateMethodUsr_FI_c> mStateMgr;

    nw4r::lyt::Picture *mpShines[COLUMN_COUNT][SHINE_COUNT];
    nw4r::lyt::Picture *mpCoinOutlines[COLUMN_COUNT][ROW_COUNT][3];
    nw4r::lyt::Picture *mpCoins[COLUMN_COUNT][ROW_COUNT][3];
    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
    LytTextBox_c *mpLevelNames[COLUMN_COUNT][ROW_COUNT];
    LytTextBox_c *mpTextBoxes[T_COUNT];

    STATE_FUNC_DECLARE(dWMStarCoin_c, Hidden);
    STATE_FUNC_DECLARE(dWMStarCoin_c, ShowWait);
    STATE_FUNC_DECLARE(dWMStarCoin_c, ShowSectionWait);
    STATE_FUNC_DECLARE(dWMStarCoin_c, Wait);
    STATE_FUNC_DECLARE(dWMStarCoin_c, HideSectionWait);
    STATE_FUNC_DECLARE(dWMStarCoin_c, HideWait);

    int mCurrentWorld;
    int mCurrentWorldIndex;
    int mWorldIndices[MAX_SECTION_COUNT];
    int mOpenWorldCount;

    bool mHasLayoutLoaded;
    bool mIsVisible;
    bool mLeftArrowActive;
    bool mRightArrowActive;
    bool mWillExit;

    static dWMStarCoin_c *build();
    static dWMStarCoin_c *m_instance;
};
#endif