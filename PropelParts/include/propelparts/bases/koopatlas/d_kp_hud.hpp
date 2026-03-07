#pragma once
#include <propelparts/game_config.h>

#if defined(KOOPATLAS_DEV_ENABLED) || defined(NEWER_MAP_HUD)
#include <propelparts/koopatlas_config.h>

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>
#include <game/mLib/m_effect.hpp>

#include <propelparts/bases/koopatlas/d_kp_common.hpp>
#include <propelparts/bases/d_world_info.hpp>
#include <propelparts/bases/koopatlas/d_tex_map_colouriser.hpp>

#ifdef KOOPATLAS_DEV_ENABLED
#include <propelparts/bases/koopatlas/d_kp_map_data.hpp>
#endif

class dKpHud_c : public dBase_c {
public:
    enum N_PANE_e {
        N_IconPos1P_00, N_IconPos2P_00,
        N_IconPos3P_00, N_IconPos4P_00,
        N_COUNT
    };

    enum P_PANE_e {
        Header_Centre, Header_Right, Footer,
        NormalExitFlag, SecretExitFlag,
        StarCoinOff0, StarCoinOff1, StarCoinOff2,
        StarCoinOn0, StarCoinOn1, StarCoinOn2,
        P_marioFace_00, P_luigiFace_00,
        P_BkinoFace_00, P_YkinoFace_00,
        Star0, Star1, Star2,
        P_COUNT
    };

    enum T_PANE_e {
        LevelName, LevelNameS,
        LevelNumber, LevelNumberS,
        WorldName, WorldNameS,
        StarCoinCounter,
        T_lifeNumber_00, T_lifeNumber_01,
        T_lifeNumber_02, T_lifeNumber_03,
        ItemsButtonInfo,
        T_COUNT
    };

    enum ANIM_e {
        ANIM_SHOW_LIVES,
        ANIM_SHOW_HEADER,
        ANIM_SHOW_FOOTER,
        ANIM_HIDE_ALL,
        ANIM_UNHIDE_ALL
    };

    enum FLAG_STATE_e {
        HIDDEN = 0,
        VISIBLE,
        FADED,
    };

    dKpHud_c();

    int create();
    int doDelete();
    int execute();
    int draw();

    void doInitialDisp();

#ifdef KOOPATLAS_DEV_ENABLED
    void enterNode(dKpNode_s *node = nullptr);
#else
    void enterNode(int world, int course);
#endif
    void exitNode();

    void offHudDisp();
    void onHudDisp();

    void offFooterDisp();
    void onFooterDisp();

    void restDisp();
    void controllerConnectCheck();

    void setFooterInfo();

private:
    void setHeaderInfo();

public:
    void playShowAnim(int id);
    void playHideAnim(int id);

private:
    nw4r::ut::WideTextWriter setupTextWriter(LytTextBox_c *textbox);
    
    void clearFlagSet(dLevelInfo_c::entry_s *entry, float *currPos);
    void collectionCoinSet(dLevelInfo_c::entry_s *entry, float *currPos);

    void drawStarEffects();

    LytBase_c mLayout;
    mEf::levelEffect_c mStarEffects[3];
    mEf::levelEffect_c mStarEffects2[3];
    dTexMapColouriser_c mHeaderCol, mFooterCol;
#ifdef KOOPATLAS_DEV_ENABLED
    dKpNode_s *mpHeaderNode;
#else
    int mWorldNo;
    int mCourseNo;
#endif

    nw4r::lyt::Pane *mpNullPanes[N_COUNT];
    nw4r::lyt::Picture *mpPicturePanes[P_COUNT];
    LytTextBox_c *mpTextBoxes[T_COUNT];

    int mControllerType;

public:
    bool mLayoutLoaded;
    bool mHideHud;

private:
    bool mInitalDispComplete;
    bool mDispHeader;
    bool mDispFooter;
    bool mFooterVisible;
    bool mDrawStarEffect;

public:
    static dKpHud_c *m_instance;
};
#endif