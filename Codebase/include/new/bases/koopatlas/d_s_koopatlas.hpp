#pragma once
#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <game/bases/d_course_select_menu.hpp>
#include <game/bases/d_dvd.hpp>
#include <game/bases/d_number_of_people_change.hpp>
#include <game/bases/d_scene.hpp>
#include <game/bases/d_stock_item.hpp>
#include <game/bases/d_yes_no_window.hpp>
#include <game/sLib/s_Phase.hpp>
#include <game/sLib/s_State.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>

// #include <common.h>
// #include <game.h>
// #include <g3dhax.h>
// #include <sfx.h>

// #include "levelinfo.h"
// #include "koopatlas/mapdata.h"
// #include "koopatlas/shop.h"
// #include "koopatlas/starcoin.h"
#include <new/bases/koopatlas/d_wm_hud.hpp>
// #include "koopatlas/pathmanager.h"
// #include "koopatlas/mapmusic.h"

#define WM_DEBUGGING
#define WM_SPAMMY_DEBUGGING

#ifdef WM_DEBUGGING
#define MapReport OSReport
#else
#define MapReport(...)
#endif

#ifdef WM_SPAMMY_DEBUGGING
#define SpammyReport OSReport
#else
#define SpammyReport(...)
#endif

void NewerMapDrawFunc();

extern "C" bool IsWarningManagerActive(); // 0x8076DB90, Newer calls this CheckIfWeCantDoStuff
int SearchForIndexOfPlayerID(int id);

#define WM_HUD WM_DANCE_PAKKUN
#define WM_SHOP WM_TOGEZO
#define WM_STARCOIN WM_GHOST

class daWMPlayer_c;
class dWMMap_c;
class dWMHud_c;
class dWMShop_c;
class dWMStarCoin_c;
class dWorldCamera_c;

class dScKoopatlas_c : public dScene_c {
public:
    dScKoopatlas_c();
    virtual ~dScKoopatlas_c();

    int create();
    int doDelete();
    int execute();

    void startMusic();

    u32 iterateMapList(u32(*callback)(u32,const char *,int,int), u32 userData, int *ptrIndex = 0);
    const char *getMapNameForIndex(int index);
    int getIndexForMapName(const char *name);

    //void startLevel(dLevelInfo_c::entry_s *level);

    bool canDoStuff();
    bool mapIsRunning();

    void showSaveWindow();

    sPhase_c mInitChain;

    sFStateMgr_c<dScKoopatlas_c, sStateMethodUsr_FI_c> mStateMgr;

    STATE_FUNC_DECLARE(dScKoopatlas_c, Limbo);
    STATE_FUNC_DECLARE(dScKoopatlas_c, ContinueWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, Normal);
    STATE_FUNC_DECLARE(dScKoopatlas_c, CompletionMsg);
    STATE_FUNC_DECLARE(dScKoopatlas_c, CompletionMsgHideWait);
    STATE_FUNC_DECLARE(dScKoopatlas_c, CSMenu);
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

    dCourseSelectMenu_c *mpCourseSelectMenu;
    void *mpSelectCursor;
    dNumberOfPeopleChange_c *mpNumPeopleChange;
    dYesNoWindow_c *mpYesNoWindow;
    void *mpContinueObj;
    dStockItem_c *mpStockItem;
    dStockItemShadow_c *mpStockItemShadow;
    void *mpEasyPairing;


    daWMPlayer_c *mpPlayer;
    dWMHud_c *mpHud;
    dWMMap_c *mpMap;
    dWMShop_c *mpShop;
    dWMStarCoin_c *mpCoins;

    int mCurrentMapID;
    const char *mMapPath;
    //dKPMapData_c mMapData;
    //dWMPathManager_c mPathManager;

    dDvd::loader_c mMapListLoader;

    bool mIsFirstPlay;
    bool mIsAfterKamekCutscene;
    bool mIsAfter8Castle;
    bool mIsEndingScene;

    bool mWarpZoneHacks;

    int mMustFixYesNoText;	

    bool mKeepMusicPlaying;

    static dScKoopatlas_c *build();
    static dScKoopatlas_c *instance; // TODO: rename to m_instance
};
#endif