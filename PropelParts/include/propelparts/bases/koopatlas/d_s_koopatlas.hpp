#pragma once
#include <propelparts/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <propelparts/koopatlas_config.h>

#include <game/bases/d_continue.hpp>
#include <game/bases/d_course_select_menu.hpp>
#include <game/bases/d_dvd.hpp>
#include <game/bases/d_easy_pairing.hpp>
#include <game/bases/d_number_of_people_change.hpp>
#include <game/bases/d_scene.hpp>
#include <game/bases/d_select_cursor.hpp>
#include <game/bases/d_stock_item.hpp>
#include <game/bases/d_yes_no_window.hpp>
#include <game/sLib/s_Phase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>
#include <game/snd/snd_audio_mgr.hpp>

#include <propelparts/bases/d_level_info.hpp>
#include <propelparts/bases/d_world_info.hpp>
#include <propelparts/bases/koopatlas/d_kp_common.hpp>
#include <propelparts/bases/koopatlas/d_kp_director.hpp>
#include <propelparts/bases/koopatlas/d_kp_hud.hpp>
#include <propelparts/bases/koopatlas/d_kp_map_data.hpp>
#include <propelparts/bases/koopatlas/d_kp_music.hpp>
#include <propelparts/bases/koopatlas/d_kp_path_manager.hpp>
#include <propelparts/bases/koopatlas/d_kp_star_coin_menu.hpp>
#include <propelparts/bases/koopatlas/d_kp_shop.hpp>

// Forward declarations
class daKpPlayer_c;
class dKpMap_c;
class dKpHud_c;
class daKpShop_c;
class dKpStarCoinMenu_c;
class dKpCamera_c;
class dKpDirector_c;

class dScKoopatlas_c : public dScene_c {
public:
    // Param bits used as behavior flags
    enum PARAM_FLAGS_e {
        AFTER_FORTRESS  = 0x20000,    ///< Is after a fortress stage.
        CHECK_AUTOWALK  = 0x40000,    ///< Check if the player should auto-walk to the next node.
        REQUEST_SAVE    = 0x80000,    ///< Should the game prompt the player to save?
        CHANGE_NODE     = 0x10000000, ///< Map is being entered from a CHANGE node.
        ENDING_SCENE    = 0x20000000, ///< Is the ending scene where the W9 warp appears.
        AFTER_KAMEK_CS  = 0x40000000, ///< Is after the Kamek cutscene. (Unused)
        IS_FIRST_PLAY   = 0x80000000  ///< Is the first load of the map.
    };

    // Used for the archive list
    struct ResItem_s {
        const char *mpPath;
        const char *mpArcName;
    };

    dScKoopatlas_c();
    virtual ~dScKoopatlas_c();

    int create();
    int doDelete();
    int execute();

    bool chkAllowExecute();
    bool chkMapIdleState();

    void startMusic();
    void startGame(dLevelInfo_c::entry_s *level);

    void onMenuDisp(dKpMusic_c::STAR_MODE_e starMode, int soundID);
    void offMenuDisp();

    void dispSavePrompt();

    // Lighting
    void setupScene();
    void loadScene(int sceneID);
    void blendScene(const char *blightName, const char *blmapName);

    // Map List parser
    u32 iterateMapList(u32(*callback)(u32,const char *,int,int), u32 userData, int *ptrIndex = 0);
    const char *getMapNameForIndex(int index);
    int getIndexForMapName(const char *name);

    STATE_FUNC_DECLARE(dScKoopatlas_c, SceneChangeWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, ContinueWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, KeyWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, MenuSelect);
    STATE_FUNC_DECLARE(dScKoopatlas_c, TitleConfirmOpenWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, TitleConfirmSelect);
    STATE_FUNC_DECLARE(dScKoopatlas_c, TitleConfirmHitWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, PlayerChangeWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, EasyPairingWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, PowerupsWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, ShopWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, CoinsWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, SaveOpen);
    STATE_FUNC_DECLARE(dScKoopatlas_c, SaveSelect);
    STATE_FUNC_DECLARE(dScKoopatlas_c, SaveWindowClose);
    STATE_FUNC_DECLARE(dScKoopatlas_c, SaveDo);
    STATE_FUNC_DECLARE(dScKoopatlas_c, SaveEndWindow);
    STATE_FUNC_DECLARE(dScKoopatlas_c, SaveEndCloseWait);
#ifndef DISABLE_QUICK_SAVE
    STATE_FUNC_DECLARE(dScKoopatlas_c, QuickSaveOpen);
    STATE_FUNC_DECLARE(dScKoopatlas_c, QuickSaveSelect);
    STATE_FUNC_DECLARE(dScKoopatlas_c, QuickSaveWindowClose);
    STATE_FUNC_DECLARE(dScKoopatlas_c, QuickSaveDo);
    STATE_FUNC_DECLARE(dScKoopatlas_c, QuickSaveEndWindow);
    STATE_FUNC_DECLARE(dScKoopatlas_c, QuickSaveEndCloseWait);
#endif
    STATE_FUNC_DECLARE(dScKoopatlas_c, SaveError);
    STATE_FUNC_DECLARE(dScKoopatlas_c, CompletionMsgWindow);
    STATE_FUNC_DECLARE(dScKoopatlas_c, CompletionMsgCloseWait);

    sPhase_c mInitChain;
    sFStateMgr_c<dScKoopatlas_c, sStateMethodUsr_FI_c> mStateMgr;

    // Retail actors
    dCourseSelectMenu_c *mpCourseSelectMenu;
    dSelectCursor_c *mpSelectCursor;
    dNumberOfPeopleChange_c *mpNumPeopleChange;
    dYesNoWindow_c *mpYesNoWindow;
    dContinue_c *mpContinue;
    dStockItem_c *mpStockItem;
    dStockItemShadow_c *mpStockItemShadow;
    dEasyPairing_c *mpEasyPairing;

    daKpPlayer_c *mpPlayer;
    dKpHud_c *mpHud;
    dKpMap_c *mpMap;
    daKpShop_c *mpShop;
    dKpStarCoinMenu_c *mpCoins;
    dKpDirector_c *mpDirector;

    dKpMapData_c mMapData;
    dKpPathManager_c mPathManager;
    dDvd::loader_c mMapListLoader;

    const char *mpMapPath;

    void *mpCurrentLight;
    void *mpCurrentLightMap;
    int mBlendStep;

    int mCurrentMapID;

    bool mDoBlendLighting;
    bool mWarpZoneHacks;
    bool mSetupCompletionMessage;
    bool mMusicPersist;

    bool mIsFirstPlay;
    bool mIsAfterKamekCutscene;
    bool mIsAfter8Castle;
    bool mIsEndingScene;

    static dScKoopatlas_c *m_instance;
    static const ResItem_s sc_arcRes[];
};
#endif