#pragma once
#include <new/game_config.h>

#if defined(KOOPATLAS_DEV_ENABLED) || defined(NEWER_MAP_HUD)
#include <new/koopatlas_config.h>

#include <game/bases/d_base.hpp>
#include <game/bases/d_lytbase.hpp>

#include <new/bases/koopatlas/d_kp_common.hpp>
#include <new/bases/d_world_info.hpp>
#include <new/bases/koopatlas/d_tex_map_colouriser.hpp>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/bases/koopatlas/d_kp_map_data.hpp>
#endif

class dKPHud_c : public dBase_c {
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

    enum ANIMS_e {
        ANIM_SHOW_LIVES,
        ANIM_SHOW_HEADER,
        ANIM_SHOW_FOOTER,
        ANIM_HIDE_ALL,
        ANIM_UNHIDE_ALL
    };

    dKPHud_c();

    int create();
    int doDelete();
    int execute();
    int draw();

    void loadInitially();

#ifdef KOOPATLAS_DEV_ENABLED
    void enteredNode(dKPNode_s *node = nullptr);
#else
    void enteredNode(int world, int course);
#endif
    void leftNode();

    void hideAll();
    void unhideAll();

    void hideFooter();
    void showFooter();

    void setupLives();
    void controllerConnectCheck();

    void loadFooterInfo();

private:
    void loadHeaderInfo();

public:
    void playShowAnim(int id);
    void playHideAnim(int id);

private:
    void drawStarEffects();

    LytBase_c mLayout;
    dTexMapColouriser_c mHeaderCol, mFooterCol;
#ifdef KOOPATLAS_DEV_ENABLED
    dKPNode_s *mpHeaderNode;
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
    static dKPHud_c *m_instance;
};
#endif