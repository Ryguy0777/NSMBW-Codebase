#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <new/koopatlas_config.h>

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

#include <new/bases/d_level_info.hpp>
#include <new/bases/d_world_info.hpp>
#include <new/bases/koopatlas/d_kp_common.hpp>
#include <new/bases/koopatlas/d_kp_director.hpp>
#include <new/bases/koopatlas/d_kp_hud.hpp>
#include <new/bases/koopatlas/d_kp_map_data.hpp>
#include <new/bases/koopatlas/d_kp_music.hpp>
#include <new/bases/koopatlas/d_kp_path_manager.hpp>
#include <new/bases/koopatlas/d_kp_star_coin_menu.hpp>
// #include "koopatlas/shop.h"

// Forward declarations
class daKPPlayer_c;
class dKPMap_c;
class dKPHud_c;
class dWMShop_c;
class dKPStarCoinMenu_c;
class dKPCamera_c;
class dKPDirector_c;

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

    dScKoopatlas_c();
    virtual ~dScKoopatlas_c();

    int create();
    int doDelete();
    int execute();

    bool chkAllowExecute();
    bool chkMapIdleState();

    void startMusic();
    void startLevel(dLevelInfo_c::entry_s *level);

    void openMenu(int starSndMode, int soundID);
    void returnToNormalState();

    void showSaveWindow();

    // Lighting
    void setupScene();
    void loadScene(int sceneID);
    void blendScene(const char *blightName, const char *blmapName);

    // Map List parser
    u32 iterateMapList(u32(*callback)(u32,const char *,int,int), u32 userData, int *ptrIndex = 0);
    const char *getMapNameForIndex(int index);
    int getIndexForMapName(const char *name);

    STATE_FUNC_DECLARE(dScKoopatlas_c, Limbo);
    STATE_FUNC_DECLARE(dScKoopatlas_c, ContinueWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, Normal);
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

    daKPPlayer_c *mpPlayer;
    dKPHud_c *mpHud;
    dKPMap_c *mpMap;
    dWMShop_c *mpShop;
    dKPStarCoinMenu_c *mpCoins;
    dKPDirector_c *mpDirector;

    dKPMapData_c mMapData;
    dWMPathManager_c mPathManager;
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
};
#endif