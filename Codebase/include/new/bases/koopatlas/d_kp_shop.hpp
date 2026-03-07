#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <game/bases/d_base_actor.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_tex_pat.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

#include <new/bases/koopatlas/d_s_koopatlas.hpp>
#include <new/bases/koopatlas/d_tex_map_colouriser.hpp>

class daKpShop_c : public dBaseActor_c {
public:
    enum ANIM_e {
        ANIM_SHOW_ALL = 0,
        ANIM_HIDE_ALL = 1,
        ANIM_ON_BUTTON = 2, // 3, 4, 5, 6, 7
        ANIM_OFF_BUTTON = 8, // 9, 10, 11, 12, 13
        ANIM_COUNT_COIN = 14,
    };

    enum ITEM_TYPE_e {
        TYPE_MUSHROOM = 0,
        TYPE_FIRE_FLOWER,
        TYPE_PROPELLER,
        TYPE_ICE_FLOWER,
        TYPE_PENGUIN,
        TYPE_MINI_SHROOM,
        TYPE_STARMAN,
        TYPE_HAMMER,
        TYPE_ONE_UP,

        TYPE_NUM
    };

    enum N_PANE_e {
        Btn00, Btn01, Btn02, Btn03, Btn1, Btn2,
        N_COUNT
    };

    enum P_PANE_e {
        BtnLeft00, BtnLeft01, BtnLeft02, BtnLeft03, BtnLeft1, BtnLeft2,
        BtnMid00, BtnMid01, BtnMid02, BtnMid03, BtnMid1, BtnMid2,
        BtnRight00, BtnRight01, BtnRight02, BtnRight03, BtnRight1, BtnRight2,
        P_COUNT
    };

    enum T_PANE_e {
        Title, TitleShadow,
        CoinCount, CoinCountShadow,
        T_COUNT
    };

    struct ItemDef_s {
        int mCost;
        int mIndex; // Start index for item(s)
        int mItemNum;
    };

    class ShopModel_c {
    public:
        void createItem(float x, float y, ITEM_TYPE_e type);
        void createLakitu(int type);
        void execute();
        void draw();
    
        void setAnm(const char *name, float rate, m3d::playMode_e playMode);

        dHeapAllocator_c mAllocator;

        nw4r::g3d::ResFile mResFile;
        m3d::mdl_c mModel;
        m3d::anmChr_c mAnmChr;
        m3d::anmTexPat_c mAnmTexPat;

        mVec2_c mPos;
        float mScaleFactor;
        float mScaleEase;

        bool mIsLakitu;
        bool mIsNotEnoughAnime;

        static const char *sc_itemResNames[TYPE_NUM][4];
        static const char *sc_lakituResNames[];
    };

    daKpShop_c();

    int create();
    int doDelete();
    int execute();
    int draw();

    void draw2D();

    void dispMenu(int shopNum);

    void loadInfo();
    void loadModels();
    void deleteModels();

    void buyItem(int item);

    int UnspentCoinSet();
    void SelectCursorSetup();

    STATE_FUNC_DECLARE(daKpShop_c, Hidden);
    STATE_FUNC_DECLARE(daKpShop_c, ShowWait);
    STATE_FUNC_DECLARE(daKpShop_c, ButtonActivateWait);
    STATE_FUNC_DECLARE(daKpShop_c, CoinCountdown);
    STATE_FUNC_DECLARE(daKpShop_c, Wait);
    STATE_FUNC_DECLARE(daKpShop_c, HideWait);

    LytBase_c mLayout;
    sFStateMgr_c<daKpShop_c, sStateMethodUsr_FI_c> mStateMgr;
    dTexMapColouriser_c mBtnLeftCol, mBtnMidCol, mBtnRightCol;

    nw4r::lyt::Pane *mpNullPanes[N_COUNT];
    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
    LytTextBox_c *mpTextBoxes[T_COUNT];

    ShopModel_c *mpItems;
    ShopModel_c *mpLakitu;

    float mScaleEase;

    int mDelay;
    int mCurrSelection;
    int mTopRowSelection;
    int mShopIdx;

    int mBoughtItemCost;
    int mCoinSpendDelay;

    bool mHasLayoutLoaded;
    bool mIsVisible;

    static const ITEM_TYPE_e Inventory[10][12];
};

#endif

