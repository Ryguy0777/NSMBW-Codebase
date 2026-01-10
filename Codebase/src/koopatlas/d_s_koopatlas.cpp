#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <kamek.h>

#include <new/bases/koopatlas/d_s_koopatlas.hpp>
#include <new/bases/koopatlas/d_kp_music.hpp>
#include <new/bases/koopatlas/d_kp_camera.hpp>
#include <new/bases/koopatlas/d_a_kp_player.hpp>
#include <new/constants/message_list.h>
#include <new/level_info_utils.hpp>

#include <constants/sound_list.h>
#include <game/bases/d_3d.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_base_actor.hpp>
#include <game/bases/d_course_select_manager.hpp>
#include <game/bases/d_effectmanager.hpp>
#include <game/bases/d_fader.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_game_key.hpp>
#include <game/bases/d_graph.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_nand_thread.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_s_game_setup.hpp>
#include <game/bases/d_s_restart_crsin.hpp>
#include <game/bases/d_save_mng.hpp>
#include <game/bases/d_WarningManager.hpp>
#include <game/bases/d_wipe_circle.hpp>
#include <game/framework/f_profile.hpp>
#include <game/mLib/m_pad.h>
#include <game/snd/snd_scene_manager.hpp>
#include <lib/egg/gfxe/eggStateGX.h>

// Reset stage exit mode after a Game Over
extern int m_exitMode__10dScStage_c;
kmBranchDefAsm(0x809216EC, 0x809216F0) {
    lis r4, m_exitMode__10dScStage_c@h
    ori r4, r4, m_exitMode__10dScStage_c@l
    li r5, 0
    stw r5, 0(r4)
    blr
}

// Fixes a crash when equipping stars
kmWriteNop(0x807B0110);

// Fix star effects on maps
kmWrite32(0x800D52F0, 0x41820010);

// Fix StockItem effects
// TODO: This makes the button effect spawn, however it is offset up and right. Player effects seem to still not appear
// Look into what causes the effects to be offset/broken entirely
kmWriteNop(0x807B0A04);

STATE_DEFINE(dScKoopatlas_c, Limbo);
STATE_DEFINE(dScKoopatlas_c, ContinueWait);
STATE_DEFINE(dScKoopatlas_c, Normal);
STATE_DEFINE(dScKoopatlas_c, MenuSelect);
STATE_DEFINE(dScKoopatlas_c, TitleConfirmOpenWait);
STATE_DEFINE(dScKoopatlas_c, TitleConfirmSelect);
STATE_DEFINE(dScKoopatlas_c, TitleConfirmHitWait);
STATE_DEFINE(dScKoopatlas_c, PlayerChangeWait);
STATE_DEFINE(dScKoopatlas_c, EasyPairingWait);
STATE_DEFINE(dScKoopatlas_c, PowerupsWait);
STATE_DEFINE(dScKoopatlas_c, ShopWait);
STATE_DEFINE(dScKoopatlas_c, CoinsWait);
STATE_DEFINE(dScKoopatlas_c, SaveOpen);
STATE_DEFINE(dScKoopatlas_c, SaveSelect);
STATE_DEFINE(dScKoopatlas_c, SaveWindowClose);
STATE_DEFINE(dScKoopatlas_c, SaveDo);
STATE_DEFINE(dScKoopatlas_c, SaveEndWindow);
STATE_DEFINE(dScKoopatlas_c, SaveEndCloseWait);
#ifndef DISABLE_QUICK_SAVE
STATE_DEFINE(dScKoopatlas_c, QuickSaveOpen);
STATE_DEFINE(dScKoopatlas_c, QuickSaveSelect);
STATE_DEFINE(dScKoopatlas_c, QuickSaveWindowClose);
STATE_DEFINE(dScKoopatlas_c, QuickSaveDo);
STATE_DEFINE(dScKoopatlas_c, QuickSaveEndWindow);
STATE_DEFINE(dScKoopatlas_c, QuickSaveEndCloseWait);
#endif
STATE_DEFINE(dScKoopatlas_c, SaveError);
STATE_DEFINE(dScKoopatlas_c, CompletionMsgWindow);
STATE_DEFINE(dScKoopatlas_c, CompletionMsgCloseWait);

dScKoopatlas_c *dScKoopatlas_c::m_instance = nullptr;

dScKoopatlas_c *dScKoopatlas_c_classInit() {
    MapReport("Creating Koopatlas\n");
    dScKoopatlas_c *c = new dScKoopatlas_c;

    MapReport("Created Koopatlas @ %p\n", c);
    dScKoopatlas_c::m_instance = c;
    return c;
}

// Replace WORLD_MAP actor
kmWritePointer(0x8098DCF0, &dScKoopatlas_c_classInit);

sPhase_c::METHOD_RESULT_e KPInitPhase_LoadSceneSnd(void*);
sPhase_c::METHOD_RESULT_e KPInitPhase_LoadKPMusic(void*);
sPhase_c::METHOD_RESULT_e StockWMInit_LoadEffects(void*); // Exists in retail sPhase, may want to look into reimplementing it here
sPhase_c::METHOD_RESULT_e KPInitPhase_LoadResources(void*);
sPhase_c::METHOD_RESULT_e KPInitPhase_EndLoading(void*);
sPhase_c::METHOD_RESULT_e KPInitPhase_LoadResources1(void*);
sPhase_c::METHOD_RESULT_e KPInitPhase_LoadResources2(void*);
sPhase_c::METHOD_RESULT_e KPInitPhase_ChkLayoutLoad(void*);
sPhase_c::METHOD_RESULT_e KPInitPhase_CreateActors(void*);
sPhase_c::METHOD_RESULT_e KPInitPhase_ChkChildProcess(void*);

sPhase_c::phaseMethod *phaseMethods[] = {
    KPInitPhase_LoadSceneSnd,
    KPInitPhase_LoadKPMusic,
    KPInitPhase_LoadResources,
    KPInitPhase_EndLoading,
    KPInitPhase_LoadResources1,
    KPInitPhase_LoadResources2,
    KPInitPhase_ChkLayoutLoad,
    KPInitPhase_CreateActors,
    KPInitPhase_ChkChildProcess
};

dScKoopatlas_c::dScKoopatlas_c() : mInitChain(phaseMethods, 9), mStateMgr(*this, StateID_Normal) {
    mpPhase = &mInitChain;
}

dScKoopatlas_c::~dScKoopatlas_c() {
    m_instance = nullptr;
}



sPhase_c::METHOD_RESULT_e KPInitPhase_LoadSceneSnd(void *ptr) {
    SpammyReport("KPInitPhase_LoadSceneSnd called\n");

    dAudio::loadSceneSnd();
    return (sPhase_c::METHOD_RESULT_e)true;
}

sPhase_c::METHOD_RESULT_e KPInitPhase_LoadKPMusic(void *ptr) {
    SpammyReport("KPInitPhase_LoadKPMusic called\n");
    bool result = dKPMusic_c::init();
    return (sPhase_c::METHOD_RESULT_e)result;
}

sPhase_c::METHOD_RESULT_e KPInitPhase_LoadResources(void *ptr) {
    SpammyReport("KPInitPhase_LoadResources called\n");

    dResMng_c::m_instance->setRes("WorldMap", "SI_kinoko", nullptr);
    dResMng_c::m_instance->setRes("WorldMap", "SI_fireflower", nullptr);
    dResMng_c::m_instance->setRes("WorldMap", "SI_iceflower", nullptr);
    dResMng_c::m_instance->setRes("WorldMap", "SI_penguin", nullptr);
    dResMng_c::m_instance->setRes("WorldMap", "SI_propeller", nullptr);
    dResMng_c::m_instance->setRes("WorldMap", "SI_star", nullptr);

    /*dResMng_c::m_instance->setRes("Object", "cobCourse", nullptr);
    dResMng_c::m_instance->setRes("Object", "I_kinoko_bundle", nullptr);
    dResMng_c::m_instance->setRes("Object", "lakitu", nullptr);
    dResMng_c::m_instance->setRes("Object", "star_coin", nullptr);
    dResMng_c::m_instance->setRes("Object", "StarRing", nullptr);*/

    return (sPhase_c::METHOD_RESULT_e)true;
}

sPhase_c::METHOD_RESULT_e KPInitPhase_EndLoading(void *ptr) {
    SpammyReport("KPInitPhase_EndLoading called\n");

    if (dResMng_c::m_instance->syncAllRes()) {
        SpammyReport("KPInitPhase_EndLoading returning false\n");
        return (sPhase_c::METHOD_RESULT_e)false;
    }

    dAudio::isLoadedSceneSnd();
    SpammyReport("KPInitPhase_EndLoading returning true\n");
    return (sPhase_c::METHOD_RESULT_e)true;
}

sPhase_c::METHOD_RESULT_e KPInitPhase_LoadResources1(void *ptr) {
    SpammyReport("KPInitPhase_LoadResources1 returning true\n");

    dScKoopatlas_c *wm = (dScKoopatlas_c*)ptr;

    //bool result = wm->mMapListLoader.request("/Maps/List.txt", 0, nullptr) != nullptr;
    //return (sPhase_c::METHOD_RESULT_e)result;
    return (sPhase_c::METHOD_RESULT_e)true;
}

sPhase_c::METHOD_RESULT_e KPInitPhase_LoadResources2(void *ptr) {
    SpammyReport("KPInitPhase_LoadResources2 returning true\n");

    dScKoopatlas_c *wm = (dScKoopatlas_c*)ptr;

    /*if (wm->mMapPath == 0) {
        wm->mMapPath = wm->getMapNameForIndex(wm->mCurrentMapID);
        if (wm->mMapPath == 0)
            wm->mMapPath = wm->getMapNameForIndex(0);
        if (!strcmp(wm->mMapPath, "/Maps/WSEL.kpbin"))
            wm->mWarpZoneHacks = true;
        else
            wm->mWarpZoneHacks = false;
        MapReport("Loading map: %s\n", wm->mMapPath);
    }

    bool result = wm->mMapData.load(wm->mMapPath);
    return (sPhase_c::METHOD_RESULT_e)result;*/
    return (sPhase_c::METHOD_RESULT_e)true;
}

sPhase_c::METHOD_RESULT_e KPInitPhase_ChkLayoutLoad(void *ptr) {
    SpammyReport("KPInitPhase_ChkLayoutLoad called\n");

    dScKoopatlas_c *wm = (dScKoopatlas_c*)ptr;

    bool success = true;
    success &= wm->mpCourseSelectMenu->mHasLayoutLoaded;
    success &= wm->mpSelectCursor->mHasLayoutLoaded;
    success &= wm->mpNumPeopleChange->mHasLayoutLoaded;
    success &= wm->mpYesNoWindow->getLayoutLoaded();
    success &= wm->mpContinue->mHasLayoutLoaded;
    success &= wm->mpStockItem->mHasLayoutLoaded;
    success &= wm->mpStockItemShadow->mHasLayoutLoaded;
    success &= wm->mpEasyPairing->mHasLayoutLoaded;

    return (sPhase_c::METHOD_RESULT_e)success;
}

sPhase_c::METHOD_RESULT_e KPInitPhase_CreateActors(void *ptr) {
    SpammyReport("KPInitPhase_CreateActors called\n");

    dScKoopatlas_c *wm = (dScKoopatlas_c*)ptr;

    // Check if we need to handle Continues
    if (dGameCom::chkContinue()) {
        SpammyReport("Showing Continues\n");
        wm->mStateMgr.changeState(wm->StateID_ContinueWait);
    } else {
        wm->mStateMgr.changeState(wm->StateID_Normal);
    }

    // Players for StockItem/CharaChange
    for (int i = 0; i < 4; i++) {
        da2DPlayer_c *obj = (da2DPlayer_c*)dBaseActor_c::construct(fProfile::WM_2D_PLAYER, wm, i, 0, 0);
        wm->mpStockItem->mpPlayers[i] = obj;
        wm->mpNumPeopleChange->mpPlayers[i] = obj;
    }

    // StockItem powerups
    for (int i = 0; i < 7; i++) {
        void *obj = dBaseActor_c::construct(fProfile::WM_ITEM, wm, i, 0, 0);
        wm->mpStockItem->mpItems[i] = obj;
    }

    // Need Player before we can set up paths
    SpammyReport("Creating player\n");
    wm->mpPlayer = (daKPPlayer_c*)fBase_c::createChild(fProfile::WM_PLAYER, wm, 0, 2);
    wm->mpPlayer->mpPyMdlMng->mpMdl->setPlayerMode(daPyMng_c::mPlayerMode[0]);
    wm->mpPlayer->bindPats();
    wm->mpPlayer->mpPyMdlMng->mpMdl->setAnm(dPyMdlBase_c::WAIT, 1.2f, 10.0f, 0.0f);

    // Since we've got all the resources, set up the path data too
    /*SpammyReport("Preparing path manager\n");
    wm->mPathManager.setup();*/

    // And put the player into position
    //dKPNode_s *cNode = wm->mPathManager.currentNode;
    //wm->mpPlayer->mPos = mVec3_c(cNode->x, -cNode->y, wm->mpPlayer->mPos.z);

    // Temp
    wm->mpPlayer->mPos = mVec3_c(0.0f, 0.0f, wm->mpPlayer->mPos.z);

    SpammyReport("Creating MAP\n");
    wm->mpMap = (dKPMap_c*)fBase_c::createChild(fProfile::WM_MAP, wm, 0, 0);

    SpammyReport("Creating HUD\n");
    wm->mpHud = (dKPHud_c*)fBase_c::createChild(fProfile::KP_HUD, wm, 0, 0);

    /*SpammyReport("creating SHOP\n");
    wm->mpShop = (dWMShop_c*)fBase_c::createChild(fProfile::WM_SHOP, wm, 0, 2);*/

    SpammyReport("Creating Star Coins Menu\n");
    wm->mpCoins = (dKPStarCoinMenu_c*)fBase_c::createChild(fProfile::KP_STAR_COIN_MENU, wm, 0, 0);

    SpammyReport("Creating Map Director\n");
    wm->mpDirector = (dKPDirector_c*)fBase_c::createChild(fProfile::WM_DIRECTOR, wm, 0, 0);

    SpammyReport("KPInitPhase_CreateActors returning true\n");
    return (sPhase_c::METHOD_RESULT_e)true;
}

sPhase_c::METHOD_RESULT_e KPInitPhase_ChkChildProcess(void *ptr) {
    SpammyReport("KPInitPhase_ChkChildProcess called\n");

    dScKoopatlas_c *wm = (dScKoopatlas_c*)ptr;

    if (wm->checkChildProcessCreateState()) {
        SpammyReport("KPInitPhase_ChkChildProcess returning false\n");
/*#ifdef WM_SPAMMY_DEBUGGING
        fBase_c *p = wm->findNextUninitialisedProcess();
        SpammyReport("Not done yet: %p [%d]\n", p, p->name);
#endif*/
        return (sPhase_c::METHOD_RESULT_e)false;
    }

    // TODO: Properly focus circle wipe on player, this doesn't work
    daKPPlayer_c *player = daKPPlayer_c::m_instance;

    dWipeCircle_c::m_instance->mCenterPos.x = player->mPos.x;
    dWipeCircle_c::m_instance->mCenterPos.y = player->mPos.y;
    dWipeCircle_c::m_instance->mUseCenterPos = true;

    // Temp until pathmanager is complete
    wm->startMusic();

    SpammyReport("KPInitPhase_ChkChildProcess returning true\n");
    return (sPhase_c::METHOD_RESULT_e)true;
}

int dScKoopatlas_c::create() {
    OSReport("KP scene param: %08x\n", mParam);
    SpammyReport("create() called\n");

    // TODO: This should probably be changed to make the scene params control the wipe used
    dFader_c::setFader(dFader_c::CIRCLE_SLOW);

    // NewerSMBW's opening cutscene loads VS effects for some reason and fragments RAM too much for some maps
    SpammyReport("Freeing effects\n");
    EffectManager_c::resetResource(EffectManager_c::EFF_VS);
    EffectManager_c::freeBreff(EffectManager_c::EFF_VS);
    EffectManager_c::freeBreft(EffectManager_c::EFF_VS);

    SpammyReport("Setting up lighting scene\n");
    setupScene();

    SpammyReport("Setting active players\n");
    for (int i = 0; i < 4; i++) {
        bool isThere = dGameCom::PlayerEnterCheck(i);
        daPyMng_c::mPlayerEntry[i] = isThere ? 1 : 0;
        if (!isThere) daPyMng_c::mCreateItem[i] = 0;
    }

    SpammyReport("Creating SELECT_CURSOR\n");
    mpSelectCursor = (dSelectCursor_c*)fBase_c::createChild(fProfile::SELECT_CURSOR, this, 0, 0);

    SpammyReport("Creating COURSE_SELECT_MENU\n");
    mpCourseSelectMenu = (dCourseSelectMenu_c*)fBase_c::createChild(fProfile::COURSE_SELECT_MENU, this, 0, 0);

    SpammyReport("Creating YES_NO_WINDOW\n");
    mpYesNoWindow = (dYesNoWindow_c*)fBase_c::createChild(fProfile::YES_NO_WINDOW, this, 0, 0);

    SpammyReport("Creating NUMBER_OF_PEOPLE_CHANGE\n");
    mpNumPeopleChange = (dNumberOfPeopleChange_c*)fBase_c::createChild(fProfile::NUMBER_OF_PEOPLE_CHANGE, this, 0, 0);

    for (int i = 0; i < 4; i++) {
        SpammyReport("Creating CC_SELECT_BASE %d\n", i);
        dCharacterChangeSelectBase_c *ccsb = (dCharacterChangeSelectBase_c*)fBase_c::createChild(fProfile::CHARACTER_CHANGE_SELECT_BASE, this, i, 0);

        SpammyReport("Creating CC_SELECT_CONTENTS %d\n", i);
        dCharacterChangeSelectContents_c *ccsc = (dCharacterChangeSelectContents_c*)fBase_c::createChild(fProfile::CHARACTER_CHANGE_SELECT_CONTENTS, this, i, 0);

        SpammyReport("Creating CC_SELECT_ARROW %d\n", i);
        void *ccsa = fBase_c::createChild(fProfile::CHARACTER_CHANGE_SELECT_ARROW, this, i, 0);

        SpammyReport("Creating CC_INDICATOR %d\n", i);
        void *cci = fBase_c::createChild(fProfile::CHARACTER_CHANGE_INDICATOR, this, i, 0);

        mpNumPeopleChange->mpSelectBases[i] = ccsb;
        mpNumPeopleChange->mpSelectContents[i] = ccsc;
        mpNumPeopleChange->mpSelectArrows[i] = ccsa;
        mpNumPeopleChange->mpIndicators[i] = cci;
    }

    SpammyReport("Creating CONTINUE\n");
    mpContinue = (dContinue_c*)fBase_c::createChild(fProfile::CONTINUE, this, 0, 0);

    SpammyReport("Creating STOCK_ITEM\n");
    mpStockItem = (dStockItem_c*)fBase_c::createChild(fProfile::STOCK_ITEM, this, 0, 0);
    SpammyReport("Creating STOCK_ITEM_SHADOW\n");
    mpStockItemShadow = (dStockItemShadow_c*)fBase_c::createChild(fProfile::STOCK_ITEM_SHADOW, this, 0, 0);
    mpStockItem->mpShadow = mpStockItemShadow;

    SpammyReport("Creating EASY_PAIRING\n");
    mpEasyPairing = (dEasyPairing_c*)fBase_c::createChild(fProfile::EASY_PAIRING, this, 0, 0);

    SpammyReport("Creating WORLD_CAMERA\n");
    fBase_c::createChild(fProfile::WORLD_CAMERA, this, 0, 0);

    SpammyReport("Setting Draw Function\n");
    dGraph_c::ms_Instance->mpDrawFunc = KoopatlasDrawFunc;


    SpammyReport("Checking cutscene/behavior params\n");
    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    mCurrentMapID = save->getCurrentWorld();
    mIsFirstPlay = (mCurrentMapID == 0) && (mParam & 0x80000000);

    mIsAfterKamekCutscene = (mParam & 0x40000000);
    if (mIsAfterKamekCutscene) {
        SpammyReport("After Kamek CS: Sending player to Map 6\n");
        mCurrentMapID = 6; // KoopaPlanet
        save->setCurrentWorld(6);
    }

    /*if ((MaybeFinishingLevel[0] == WORLD_8 && MaybeFinishingLevel[1] == STAGE_CASTLE_2) && save->isCourseDataFlag(7, 24, dMj2dGame_c::GOAL_NORMAL)) {
        SpammyReport("After 8-Castle: Sending player to Map 7\n");
        mCurrentMapID = 7; // KoopaPlanetUnd
        save->setCurrentWorld(7);
        mIsAfter8Castle = true;
    }*/

    mIsEndingScene = (mParam & 0x20000000);
    if (mIsEndingScene) {
        SpammyReport("Ending Scene: Sending player to Map 0 Node 0\n");
        mCurrentMapID = 0;
        save->setCurrentWorld(0);
        save->setCurrentPathNode(0);
    }

    SndSceneMgr::sInstance->closeWorldSelect();
    SpammyReport("create() completed\n");
    return true;
}

int dScKoopatlas_c::doDelete() {
    if (!mMusicPersist)
        dKPMusic_c::m_instance->stop();

    m3d::removeLightMgr(0);
    m3d::removeLightMgr(1);

    EffectManager_c::courseOut();
    EffectManager_c::resetResource(EffectManager_c::EFF_MAP);
    EffectManager_c::freeBreff(EffectManager_c::EFF_MAP);
    EffectManager_c::freeBreft(EffectManager_c::EFF_MAP);

    dResMng_c::m_instance->getResP()->deleteRes("SI_kinoko");
    dResMng_c::m_instance->getResP()->deleteRes("SI_fireflower");
    dResMng_c::m_instance->getResP()->deleteRes("SI_iceflower");
    dResMng_c::m_instance->getResP()->deleteRes("SI_penguin");
    dResMng_c::m_instance->getResP()->deleteRes("SI_propeller");
    dResMng_c::m_instance->getResP()->deleteRes("SI_star");

    dResMng_c::m_instance->getResP()->deleteRes("cobCourse");
    dResMng_c::m_instance->getResP()->deleteRes("I_kinoko_bundle");
    dResMng_c::m_instance->getResP()->deleteRes("lakitu");
    dResMng_c::m_instance->getResP()->deleteRes("star_coin");
    dResMng_c::m_instance->getResP()->deleteRes("StarRing");

    mMapListLoader.freeResouce();

    EffectManager_c::courseOut();
    return true;
}

int dScKoopatlas_c::execute() {
    dKPMusic_c::m_instance->execute();
    if (!chkAllowExecute()) {
        return true;
    }

    //SpammyReport("Executing mStateMgr: %s\n", mStateMgr.getStateID()->name());
    mStateMgr.executeState();
    return true;
}

bool dScKoopatlas_c::chkAllowExecute() {
    if (dGameCom::isGameStop(dGameCom::GAME_STOP_ANY)) return false;
    if (dWarningManager_c::isWarningCheck()) return false;
    if (mStateMgr.getStateID() == &StateID_Limbo) return false;
    return true;
}

bool dScKoopatlas_c::chkMapIdleState() {
    if (dGameCom::isGameStop(dGameCom::GAME_STOP_ANY)) return false;
    if (dWarningManager_c::isWarningCheck()) return false;
    if (mStateMgr.getStateID() != &StateID_Normal) return false;
    return true;
}

void dScKoopatlas_c::startMusic() {
    u32 worldIdx = dSaveMng_c::m_instance->getSaveGame(-1)->mWorldInfoIdx;
    int musicID = dWorldInfo_c::m_instance.getWorld(worldIdx)->mMusicID;
    dKPMusic_c::m_instance->start(musicID);
}

void dScKoopatlas_c::startLevel(dLevelInfo_c::entry_s *entry) {
    dInfo_c::StartGameInfo_s startInfo;
    startInfo.mMovieType = 0;
    startInfo.mEntrance = 0xFF;
    startInfo.mArea = 0;
    startInfo.mIsReplay = false;
    startInfo.mScreenType = 0;

    startInfo.mWorld1 = entry->mWorldSlot;
    startInfo.mWorld2 = entry->mWorldSlot;
    startInfo.mLevel1 = entry->mLevelSlot;
    startInfo.mLevel2 = entry->mLevelSlot;

    dFader_c::setFader(dFader_c::MARIO);
    dInfo_c::m_instance->startGame(startInfo);
}

void dScKoopatlas_c::openMenu(int starSndMode, int soundID) {
    dKPMusic_c::m_instance->updTrackVolume(true);
    dKPMusic_c::m_instance->updStarVolume(starSndMode);
    if (soundID > -1)
        SndAudioMgr::sInstance->startSystemSe(soundID, 1);
    mpHud->hideAll();
}

void dScKoopatlas_c::returnToNormalState() {
    dKPMusic_c::m_instance->updTrackVolume(false);
    dKPMusic_c::m_instance->updStarVolume(0);
    mpHud->unhideAll();
    mStateMgr.changeState(StateID_Normal);
}

void dScKoopatlas_c::showSaveWindow() {
    mpHud->hideAll();
    mStateMgr.changeState(StateID_SaveOpen);
    mpYesNoWindow->setType(dYesNoWindow_c::SAVE);
    mpYesNoWindow->setIsActive(true);
}

/**********************************************************************/
// LIGHTING SYSTEM
/**********************************************************************/
void dScKoopatlas_c::setupScene() {
    d3d::createLightMgr(EGG::Heap::sCurrentHeap, 36, 8, 2, false, 0);
    loadScene(0);
    d3d::createLightMgr(EGG::Heap::sCurrentHeap, 36, 8, 2, true, 1);
    loadScene(1);
}

void dScKoopatlas_c::loadScene(int sceneID) {
    EGG::LightManager *lightMgr = m3d::getLightMgr(sceneID);

    nw4r::g3d::ResFile res = dResMng_c::m_instance->getRes("Env_world", "scene/scene.brres");
    nw4r::g3d::ResAnmScn anmScn = res.GetResAnmScn("MainSelect");
    anmScn.Bind(anmScn);

    lightMgr->LoadScnLightInner(anmScn, 0.0f, -1, -3);

    if (sceneID == 0) { // Main lighting
        nw4r::g3d::ResFile resLight = dResMng_c::m_instance->getRes("Env_world", "light/W8.blight");
        lightMgr->LoadBinary(resLight.ptr());

        nw4r::g3d::ResFile resLightMap = dResMng_c::m_instance->getRes("Env_world", "light/W8.blmap");
        lightMgr->ltMgr->LoadBinary(resLightMap.ptr());

    } else if (sceneID == 1) { // Layout model lighting
        nw4r::g3d::ResFile resLight = dResMng_c::m_instance->getRes("Env_world", "light/Layout3D.blight");
        lightMgr->LoadBinary(resLight.ptr());

        nw4r::g3d::ResFile resLightMap = dResMng_c::m_instance->getRes("Env_world", "light/Layout3D.blmap");
        lightMgr->ltMgr->LoadBinary(resLightMap.ptr());

        EGG::FogManager *fogMgr = m3d::getFogMgr(sceneID);
        nw4r::g3d::ResFile resFog = dResMng_c::m_instance->getRes("Env_world", "fog/Layout3D.bfog");
        fogMgr->LoadBinary(resFog.ptr());
    }
}

void dScKoopatlas_c::blendScene(const char *blightName, const char *blmapName) {
    if (mDoBlendLighting) {
        MapReport("Blending map lighting\n");
        EGG::LightManager *lightMgr = m3d::getLightMgr(0);

        nw4r::g3d::ResFile resLight = dResMng_c::m_instance->getRes("Env_world", blightName);
        nw4r::g3d::ResFile resLightMap = dResMng_c::m_instance->getRes("Env_world", blmapName);

        float ratio = (float)mBlendStep / 60.0f;
        lightMgr->SetBinaryInner(mpCurrentLight, resLight.ptr(), ratio);
        SpammyReport("Blend: mBlendStep is %d, ratio %f\n", mBlendStep, ratio);
        mBlendStep++;

        if (mBlendStep > 60) {
            // TODO: Find out if lightmaps support SetBinaryInner
            lightMgr->ltMgr->LoadBinary(resLightMap.ptr());

            mBlendStep = 0;
            mDoBlendLighting = false;
            mpCurrentLight = resLight.ptr();
            mpCurrentLightMap = resLightMap.ptr();
        }
    }
}

/**********************************************************************/
// MAP LIST PARSER
/**********************************************************************/
u32 dScKoopatlas_c::iterateMapList(u32(*callback)(u32,const char *,int,int), u32 userData, int *ptrIndex) {
    u8 *ptr = (u8*)mMapListLoader.GetBuffer();
    u8 *strStart = ptr;
    u8 *end = ptr + mMapListLoader.getSize();
    int index = 0;

    while (true) {
        u8 chr = *ptr;
        if (chr == 13) {
            *ptr = 0;
            ++ptr;
            continue;
        }

        if (chr == 10 || chr == 0 || ptr >= end) {
            if (strStart == ptr) {
                // Blank string, ignore
                ++strStart;
                ++ptr;
                continue;
            }

            // Change the linefeed to a NUL so we can use the line as a C string later
            if (ptr < end)
                *ptr = 0;

            u32 ret = callback(userData, (const char*)strStart, ptr - strStart, index);
            if (ptrIndex)
                *ptrIndex = index;
            if (ret > 0)
                return ret;

            strStart = ++ptr;
            ++index;

            if (ptr >= end)
                break;

        } else {
            ++ptr;
        }
    }

    return 0;
}

static u32 _cb_getIndex(u32 userData, const char *str, int size, int index) {
    if (index == userData)
        return (u32)str;
    else
        return 0;
}

static u32 _cb_searchName(u32 userData, const char *str, int size, int index) {
    if (strncmp(str, (const char*)userData, size) == 0)
        return (u32)(index+1);
    else
        return 0;
}

const char *dScKoopatlas_c::getMapNameForIndex(int index) {
    return (const char *)iterateMapList(&_cb_getIndex, (u32)index, 0);
}

int dScKoopatlas_c::getIndexForMapName(const char *name) {
    return ((int)iterateMapList(&_cb_searchName, (u32)name, 0)) - 1;
}

/**********************************************************************/
// KOOPATLAS STATES
/**********************************************************************/

/**********************************************************************/
// StateID_Limbo : Waiting to return to the Title Screen
void dScKoopatlas_c::initializeState_Limbo() { }
void dScKoopatlas_c::executeState_Limbo() { }
void dScKoopatlas_c::finalizeState_Limbo() { }

/**********************************************************************/
// StateID_ContinueWait : Waiting for the Continue screen to end
void dScKoopatlas_c::initializeState_ContinueWait() {
    dInfo_c::m_instance->mDrawEffectsForMapLayouts = true;
    mpContinue->mIsVisible = true;
    mpContinue->mWillOpen = true;
    mpContinue->mIsGameOver = false;
}
void dScKoopatlas_c::executeState_ContinueWait() {
    if (mpContinue->mWillClose) {
        mpContinue->mIsVisible = false;
        mpContinue->mWillOpen = false;
        mpContinue->mIsGameOver = false;

        static const int things[] = {0,1,3,2,4};
        for (int i = 0; i < 4; i++) {
            int idx = SearchForIndexOfPlayerID(things[i]);
            daPyMng_c::mRest[daPyMng_c::mPlayerType[idx]] = mpContinue->mLives[i];
        }

        mStateMgr.changeState(StateID_Normal);
    }
}
void dScKoopatlas_c::finalizeState_ContinueWait() {
    dInfo_c::m_instance->mDrawEffectsForMapLayouts = false;
}
static int completionMsgIdx = 0;
/**********************************************************************/
// StateID_Normal : Main/idle state, waiting for input
void dScKoopatlas_c::initializeState_Normal() { }
void dScKoopatlas_c::executeState_Normal() {
    /*if (mPathManager.completionMessagePending) {
        MapReport("Going to set CompletionMsg\n");
        mStateMgr.changeState(StateID_CompletionMsg);
        return;
    }

    if (mPathManager.doingThings())
        return;*/

    int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;

    if (pressed & WPAD_BUTTON_1) {
        openMenu(2, SE_SYS_DECIDE);
        mpStockItem->mIsVisible = true;
        mStateMgr.changeState(StateID_PowerupsWait);
    } else if (pressed & WPAD_BUTTON_PLUS) {
        openMenu(1, SE_SYS_PAUSE);
        mpCourseSelectMenu->mIsVisible = true;
        mStateMgr.changeState(StateID_MenuSelect);
#ifdef KP_PATH_QUICK_UNLOCK
    } else if (pressed & WPAD_BUTTON_MINUS) {
        mPathManager.unlockAllPaths(2);
    } else if (pressed & WPAD_BUTTON_A) {
        mPathManager.unlockAllPaths(0);
        dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
        for (int w = 0; w < WORLD_COUNT; w++)
            for (int l = 0; l < STAGE_COUNT; l++)
                save->onCourseDataFlag(w, l, save->GOAL_MASK);
#endif
    } else if (pressed & WPAD_BUTTON_A) {
        //dKPMusic_c::m_instance->play(2);
        mpDirector->toggleCaptureDisp(true);
        completionMsgIdx++;
        if (completionMsgIdx > 7) completionMsgIdx = 7;
        OSReport("Inc message to %d\n", completionMsgIdx);
    } else if (pressed & WPAD_BUTTON_B) {
        mpDirector->toggleCaptureDisp(false);
        completionMsgIdx--;
        if (completionMsgIdx < 0) completionMsgIdx = 0;
        OSReport("Dec message to %d\n", completionMsgIdx);
    } else if (pressed & WPAD_BUTTON_MINUS) {
        //dKPMusic_c::m_instance->play(1);
        OSReport("Toggling completion message %d\n", completionMsgIdx);
        mStateMgr.changeState(StateID_CompletionMsgWindow);
    }
}
void dScKoopatlas_c::finalizeState_Normal() { }

/**********************************************************************/
// StateID_MenuSelect : Waiting for input on the Course Select Menu
void dScKoopatlas_c::initializeState_MenuSelect() { }
void dScKoopatlas_c::executeState_MenuSelect() {
    if (!mpCourseSelectMenu->mIsVisible) {
        if (mpCourseSelectMenu->mChoiceMade) {
            dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);

            switch (mpCourseSelectMenu->mCurrentSelection) {
                case 0: // Star Coins
                    MapReport("Star Coins was pressed\n");
                    mpCoins->dispMenu();
                    mStateMgr.changeState(StateID_CoinsWait);
                    break;

                case 1: // Add/Drop Players
                    MapReport("Add/Drop Players was pressed\n");
                    SndAudioMgr::sInstance->startSystemSe(SE_SYS_DIALOGUE_IN, 1);
                    mStateMgr.changeState(StateID_PlayerChangeWait);
                    mpNumPeopleChange->mIsVisible = true;
                    mPad::setGetWPADInfoInterval(10);
                    break;

                case 2: // Save or Quick Save
                    MapReport("Save or Quick Save was pressed\n");
#ifndef DISABLE_QUICK_SAVE
                    if (dSaveMng_c::m_instance->getSaveGame(-1)->getGameCompletion() & 2) {
#endif
                        mStateMgr.changeState(StateID_SaveOpen);
                        mpYesNoWindow->setType(dYesNoWindow_c::SAVE);
                        mpYesNoWindow->setIsActive(true);

#ifndef DISABLE_QUICK_SAVE
                    } else {
                        mStateMgr.changeState(StateID_QuickSaveOpen);
                        mpYesNoWindow->setType(dYesNoWindow_c::QUICK_SAVE);
                        mpYesNoWindow->setIsActive(true);

                    }
#endif
                    break;

                case 3: // Title Screen
                    MapReport("Title Screen was pressed\n");
                    mStateMgr.changeState(StateID_TitleConfirmOpenWait);
                    mpYesNoWindow->setIsActive(true);
                    mpYesNoWindow->setType(dYesNoWindow_c::RETURN_TITLE);
                    break;
            }

        } else {
            returnToNormalState();
        }
    }
}
void dScKoopatlas_c::finalizeState_MenuSelect() { }

/**********************************************************************/
// StateID_TitleConfirmOpenWait : Waiting for the "Go to Title Screen" YesNoWindow to
// finish opening
void dScKoopatlas_c::initializeState_TitleConfirmOpenWait() { }
void dScKoopatlas_c::executeState_TitleConfirmOpenWait() {
    //
    if (!mpYesNoWindow->mIsAnimating) {
        mStateMgr.changeState(StateID_TitleConfirmSelect);
    }
}
void dScKoopatlas_c::finalizeState_TitleConfirmOpenWait() { }

/**********************************************************************/
// StateID_TitleConfirmSelect : Let the user choose an option on the
// "Go to Title Screen" YesNoWindow.
void dScKoopatlas_c::initializeState_TitleConfirmSelect() { }
void dScKoopatlas_c::executeState_TitleConfirmSelect() {
    int nowPressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;

    if (nowPressed & WPAD_BUTTON_UP) { // Select "OK!"
        mpYesNoWindow->setCursorPos(1);
    } else if (nowPressed & WPAD_BUTTON_DOWN) { // Select "Cancel"
        mpYesNoWindow->setCursorPos(0);
    } else if (nowPressed & (WPAD_BUTTON_A | WPAD_BUTTON_2)) { // Pick the current option
        mpYesNoWindow->mHitButton = true;
        if (mpYesNoWindow->getCursorPos() != 1)
            mpYesNoWindow->mKeepOpen = true;
        mStateMgr.changeState(StateID_TitleConfirmHitWait);
    } else {
        if (dGameCom::chkCancelButton(0)) { // Cancel using B or 1
            mpYesNoWindow->mCancel = true;
            mpYesNoWindow->setCursorPos(1);
            mStateMgr.changeState(StateID_TitleConfirmHitWait);
        }
    }

}
void dScKoopatlas_c::finalizeState_TitleConfirmSelect() { }

/**********************************************************************/
// StateID_TitleConfirmHitWait : Process the user's chosen option on
// the "Go to Title Screen" YesNoWindow
void dScKoopatlas_c::initializeState_TitleConfirmHitWait() { }
void dScKoopatlas_c::executeState_TitleConfirmHitWait() {
    if (!mpYesNoWindow->mIsAnimating) {
        if (mpYesNoWindow->getCursorPos() == 1) {
            returnToNormalState();
        } else {
            mStateMgr.changeState(StateID_Limbo);
            dScRestartCrsin_c::startTitle(false, 0);
        }
    }
}
void dScKoopatlas_c::finalizeState_TitleConfirmHitWait() { }

/**********************************************************************/
// StateID_PlayerChangeWait : Wait for the user to do something on the
// Add/Drop Players screen
void dScKoopatlas_c::initializeState_PlayerChangeWait() {
    dKPMusic_c::m_instance->updStarVolume(2);
}
void dScKoopatlas_c::executeState_PlayerChangeWait() {
    int nowPressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;

    if (mpNumPeopleChange->mAllowEasyPairing) {
        if (nowPressed & WPAD_BUTTON_PLUS) {
            // Activate Easy Pairing
            mpNumPeopleChange->mIsEasyPairingActive = true;

            for (int i = 0; i < 4; i++) {
                mpNumPeopleChange->mpPlayers[i]->mIsEasyPairingActive = true;
                mpNumPeopleChange->mpSelectBases[i]->mIsEasyPairingActive = true;
                mpNumPeopleChange->mpSelectContents[i]->mIsEasyPairingActive = true;
            }

            mpEasyPairing->mIsVisible = true;
            dKPMusic_c::m_instance->updStarVolume(1);
            mStateMgr.changeState(StateID_EasyPairingWait);
        }
    } else {
        if (!mpNumPeopleChange->mIsVisible) {
            for (int i = 0; i < 4; i++) {
                bool isThere = dGameCom::PlayerEnterCheck(i);
                daPyMng_c::mPlayerEntry[i] = isThere ? 1 : 0;
                if (!isThere) daPyMng_c::mCreateItem[i] = 0;
            }

            returnToNormalState();
        }
    }
}
void dScKoopatlas_c::finalizeState_PlayerChangeWait() { }

/**********************************************************************/
// StateID_EasyPairingWait : Wait for the user to exit Easy Pairing
void dScKoopatlas_c::initializeState_EasyPairingWait() { }
void dScKoopatlas_c::executeState_EasyPairingWait() {
    if (!mpEasyPairing->mIsVisible) {
        mpNumPeopleChange->mIsEasyPairingActive = false;

        for (int i = 0; i < 4; i++) {
            mpNumPeopleChange->mpPlayers[i]->mIsEasyPairingActive = false;
            mpNumPeopleChange->mpSelectBases[i]->mIsEasyPairingActive = false;
            mpNumPeopleChange->mpSelectContents[i]->mIsEasyPairingActive = false;
        }

        mStateMgr.changeState(StateID_PlayerChangeWait);
        mPad::setGetWPADInfoInterval(10);
    }
}
void dScKoopatlas_c::finalizeState_EasyPairingWait() { }

/**********************************************************************/
// StateID_PowerupsWait : Wait for the user to exit the Items screen
void dScKoopatlas_c::initializeState_PowerupsWait() { }
void dScKoopatlas_c::executeState_PowerupsWait() {
    if (!mpStockItem->mIsVisible) {
        mpPlayer->mpPyMdlMng->mpMdl->setPlayerMode(daPyMng_c::mPlayerMode[0]);
        //mpPlayer->bindPats();

        returnToNormalState();
    }
}
void dScKoopatlas_c::finalizeState_PowerupsWait() { }


/**********************************************************************/
// StateID_ShopWait : Wait for the user to exit the Shop window
void dScKoopatlas_c::initializeState_ShopWait() { }
void dScKoopatlas_c::executeState_ShopWait() {
    //if (!mpShop->visible) {
    //    returnToNormalState();
    //}
}
void dScKoopatlas_c::finalizeState_ShopWait() { }


/**********************************************************************/
// StateID_StarCoin : Wait for the user to exit the Star Coins menu
void dScKoopatlas_c::initializeState_CoinsWait() { }
void dScKoopatlas_c::executeState_CoinsWait() {
    if (!mpCoins->mIsVisible) {
        returnToNormalState();
    }
}
void dScKoopatlas_c::finalizeState_CoinsWait() { }

/**********************************************************************/
// StateID_SaveOpen : Waiting for the "Save?" YesNoWindow to open
void dScKoopatlas_c::initializeState_SaveOpen() { }
void dScKoopatlas_c::finalizeState_SaveOpen() { }
void dScKoopatlas_c::executeState_SaveOpen() {
    if (!mpYesNoWindow->mIsAnimating) {
        mStateMgr.changeState(StateID_SaveSelect);
    }
}

/**********************************************************************/
// StateID_SaveSelect : Let the user choose an option on the
// "Save?" YesNoWindow
void dScKoopatlas_c::initializeState_SaveSelect() { }
void dScKoopatlas_c::executeState_SaveSelect() {
    int nowPressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;

    if (nowPressed & WPAD_BUTTON_UP) { // Select "OK!"
        mpYesNoWindow->setCursorPos(1);
    } else if (nowPressed & WPAD_BUTTON_DOWN) { // Select "Cancel"
        mpYesNoWindow->setCursorPos(0);
    } else if (nowPressed & (WPAD_BUTTON_A | WPAD_BUTTON_2)) { // Pick the current option
        mpYesNoWindow->mHitButton = true;
        if (mpYesNoWindow->getCursorPos() != 1)
            mpYesNoWindow->mHideBG = true;
        mStateMgr.changeState(StateID_SaveWindowClose);
    } else { // Cancel using B or 1
        if (dGameCom::chkCancelButton(0)) {
            mpYesNoWindow->mCancel = true;
            mpYesNoWindow->setCursorPos(1);
            mStateMgr.changeState(StateID_SaveWindowClose);
        }
    }
}
void dScKoopatlas_c::finalizeState_SaveSelect() { }

/**********************************************************************/
// StateID_SaveWindowClose : Process the user's chosen option on the
// "Save?" YesNoWindow
void dScKoopatlas_c::initializeState_SaveWindowClose() { }
void dScKoopatlas_c::executeState_SaveWindowClose() {
    if (!mpYesNoWindow->getIsActive()) {
        if (mpYesNoWindow->getCursorPos() == 1) {
            returnToNormalState();
        } else {
            mStateMgr.changeState(StateID_SaveDo);
            dCourseSelectManager_c::m_instance->saveGame(false);
        }
    }
}
void dScKoopatlas_c::finalizeState_SaveWindowClose() { }

/**********************************************************************/
// StateID_SaveDo : Save the game
void dScKoopatlas_c::initializeState_SaveDo() { }
void dScKoopatlas_c::executeState_SaveDo() {
    if (!dSaveMng_c::isNandBusy()) {
        if (dNandThread_c::m_instance->mCurrentError == 0) {
            mpYesNoWindow->setType(dYesNoWindow_c::SAVED);
            mpYesNoWindow->setIsActive(true);
            mStateMgr.changeState(StateID_SaveEndWindow);
        } else {
            mStateMgr.changeState(StateID_SaveError);
        }
    }
}
void dScKoopatlas_c::finalizeState_SaveDo() { }

/**********************************************************************/
// StateID_SaveEndWindow : Handle the Save End window
void dScKoopatlas_c::initializeState_SaveEndWindow() { }
void dScKoopatlas_c::executeState_SaveEndWindow() {
    if (!mpYesNoWindow->mIsAnimating) {
        int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;
        if (pressed & (WPAD_BUTTON_A | WPAD_BUTTON_2)) {
            mpYesNoWindow->mHitButton = true;
            mStateMgr.changeState(StateID_SaveEndCloseWait);
        }
    }
}
void dScKoopatlas_c::finalizeState_SaveEndWindow() { }

/**********************************************************************/
// StateID_SaveEndCloseWait : Wait for the Save End window to close
void dScKoopatlas_c::initializeState_SaveEndCloseWait() { }
void dScKoopatlas_c::executeState_SaveEndCloseWait() {
    if (!mpYesNoWindow->mIsAnimating) {
        returnToNormalState();
    }
}
void dScKoopatlas_c::finalizeState_SaveEndCloseWait() { }

#ifndef DISABLE_QUICK_SAVE
/**********************************************************************/
// StateID_QuickSaveOpen : Waiting for the "Save?" YesNoWindow to open
void dScKoopatlas_c::initializeState_QuickSaveOpen() { }
void dScKoopatlas_c::executeState_QuickSaveOpen() {
    if (!mpYesNoWindow->mIsAnimating) {
        mStateMgr.changeState(StateID_QuickSaveSelect);
    }
}
void dScKoopatlas_c::finalizeState_QuickSaveOpen() { }

/**********************************************************************/
// StateID_QuickSaveSelect : Let the user choose an option on the
// "Save?" YesNoWindow
void dScKoopatlas_c::initializeState_QuickSaveSelect() { }
void dScKoopatlas_c::executeState_QuickSaveSelect() {
    int nowPressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;

    if (nowPressed & WPAD_BUTTON_UP) { // Select "OK!"
        mpYesNoWindow->setCursorPos(1);
    } else if (nowPressed & WPAD_BUTTON_DOWN) { // Select "Cancel"
        mpYesNoWindow->setCursorPos(0);
    } else if (nowPressed & (WPAD_BUTTON_A | WPAD_BUTTON_2)) { // Pick the current option
        mpYesNoWindow->mHitButton = true;
        if (mpYesNoWindow->getCursorPos() != 1)
            mpYesNoWindow->mHideBG = true;
        mStateMgr.changeState(StateID_QuickSaveWindowClose);
    } else { // Cancel using B or 1
        if (dGameCom::chkCancelButton(0)) {
            mpYesNoWindow->mCancel = true;
            mpYesNoWindow->setCursorPos(1);
            mStateMgr.changeState(StateID_QuickSaveWindowClose);
        }
    }
}
void dScKoopatlas_c::finalizeState_QuickSaveSelect() { }

/**********************************************************************/
// StateID_QuickSaveWindowClose : Process the user's chosen option on
// the "Save?" YesNoWindow
void dScKoopatlas_c::initializeState_QuickSaveWindowClose() { }
void dScKoopatlas_c::executeState_QuickSaveWindowClose() {
    if (!mpYesNoWindow->getIsActive()) {
        if (mpYesNoWindow->getCursorPos() == 1) {
            returnToNormalState();
        } else {
            mStateMgr.changeState(StateID_QuickSaveDo);
            dCourseSelectManager_c::m_instance->saveGame(true);
        }
    }
}
void dScKoopatlas_c::finalizeState_QuickSaveWindowClose() { }

/**********************************************************************/
// StateID_QuickSaveDo : Save the game
void dScKoopatlas_c::initializeState_QuickSaveDo() { }
void dScKoopatlas_c::executeState_QuickSaveDo() {
    if (!dSaveMng_c::isNandBusy()) {
        if (dNandThread_c::m_instance->mCurrentError == 0) {
            mpYesNoWindow->setType(dYesNoWindow_c::QUICK_SAVED);
            mpYesNoWindow->setIsActive(true);
            mStateMgr.changeState(StateID_QuickSaveEndWindow);
        } else {
            mStateMgr.changeState(StateID_SaveError);
        }
    }
}
void dScKoopatlas_c::finalizeState_QuickSaveDo() { }

/**********************************************************************/
// StateID_QuickSaveEndWindow : Handle the Save End window
void dScKoopatlas_c::initializeState_QuickSaveEndWindow() { }
void dScKoopatlas_c::executeState_QuickSaveEndWindow() {
    if (!mpYesNoWindow->mIsAnimating) {
        int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;
        if (pressed & (WPAD_BUTTON_A | WPAD_BUTTON_2)) {
            mpYesNoWindow->mHitButton = true;
            mStateMgr.changeState(StateID_QuickSaveEndCloseWait);
        }
    }
}
void dScKoopatlas_c::finalizeState_QuickSaveEndWindow() { }

/**********************************************************************/
// StateID_QuickSaveEndCloseWait : Wait for Save End window to close
void dScKoopatlas_c::initializeState_QuickSaveEndCloseWait() { }
void dScKoopatlas_c::executeState_QuickSaveEndCloseWait() {
    if (!mpYesNoWindow->mIsAnimating) {
        if (mpYesNoWindow->getCursorPos() == 1) {
            returnToNormalState();
        } else {
            mStateMgr.changeState(StateID_Limbo);
            dScRestartCrsin_c::startTitle(false, 0);
        }
    }
}
void dScKoopatlas_c::finalizeState_QuickSaveEndCloseWait() { }
#endif

/**********************************************************************/
// StateID_SaveError : Display a save error warning
void dScKoopatlas_c::initializeState_SaveError() {
    int error = dNandThread_c::m_instance->mCurrentError;
    dWarningManager_c::m_instance->mIsSaveError = true;
    dWarningManager_c::m_instance->mCurrentSaveError = error;
}
void dScKoopatlas_c::executeState_SaveError() {
    if (!dWarningManager_c::m_instance->mIsSaveError) {
        returnToNormalState();
    }
}
void dScKoopatlas_c::finalizeState_SaveError() { }

// TODO: Move into PathManager once ported
enum CompletionType_e {
    CMP_MSG_NULL = 0,
    CMP_MSG_COINS,
    CMP_MSG_EXITS,
    CMP_MSG_WORLD,
    CMP_MSG_GLOBAL_COINS_EXC_W9,
    CMP_MSG_GLOBAL_COINS,
    CMP_MSG_GLOBAL_EXITS,
    CMP_MSG_EVERYTHING
};

/**********************************************************************/
// StateID_CompletionMsgWindow : Display a completion message
void dScKoopatlas_c::initializeState_CompletionMsgWindow() {
    //MapReport("CompletionMsg beginning with type %d\n", mPathManager.completionMessageType);
    //mpYesNoWindow->setType(yesNoTypes[mPathManager.completionMessageType]);
    mpYesNoWindow->setType(dYesNoWindow_c::SAVE_DATA_CREATED); // Actual window type doesn't matter
    mpYesNoWindow->setIsActive(true);
    mSetupCompletionMessage = true;
}
void dScKoopatlas_c::executeState_CompletionMsgWindow() {
    if (mSetupCompletionMessage) {
        MsgRes_c *msgRes = dMessage_c::getMesRes();
        LytTextBox_c *T_questionS_00 = mpYesNoWindow->getTextBox(0);
        LytTextBox_c *T_question_00 = mpYesNoWindow->getTextBox(1);
        //int type = mPathManager.completionMessageType;
        int type = completionMsgIdx;

        const wchar_t *message = dMessage_c::getMsg(BMG_CATEGORY_KOOPATLAS, type);

        T_questionS_00->setMessage(msgRes, BMG_CATEGORY_KOOPATLAS, type, 0);
        T_question_00->setMessage(msgRes, BMG_CATEGORY_KOOPATLAS, type, 0);

        // Append a world name if needed
        if (type >= CMP_MSG_COINS && type <= CMP_MSG_WORLD) {
            wchar_t nameBuffer[36];

            wcscpy(nameBuffer, getKoopatlasWorldName(dGameCom::rndInt(8)));
            int offs = wcslen(nameBuffer);
            nameBuffer[offs] = L'!';

            u16 strLen = T_questionS_00->GetStringBufferLength();
            T_question_00->SetString(nameBuffer, strLen);
            T_questionS_00->SetString(nameBuffer, strLen);
        }

        mSetupCompletionMessage = false;
    }

    // Process window input
    if (!mpYesNoWindow->mIsAnimating) {
        int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;
        if (pressed & (WPAD_BUTTON_A | WPAD_BUTTON_2)) {
            mpYesNoWindow->mHitButton = true;
            mStateMgr.changeState(StateID_CompletionMsgCloseWait);
        }
    }
}
void dScKoopatlas_c::finalizeState_CompletionMsgWindow() {
    //mPathManager.completionMessagePending = false;
    //mPathManager.completionMessageType = 0;
}

/**********************************************************************/
// StateID_CompletionMsgCloseWait : Wait for the completion message
// window to close
void dScKoopatlas_c::initializeState_CompletionMsgCloseWait() { }
void dScKoopatlas_c::executeState_CompletionMsgCloseWait() {
    if (!mpYesNoWindow->getIsActive())
        returnToNormalState();
}
void dScKoopatlas_c::finalizeState_CompletionMsgCloseWait() { }


void KoopatlasDrawFunc() {
    m3d::reset();
    m3d::setCurrentCamera(0);
    m3d::screenEffectReset(m3d::getCurrentCameraID(), dKPCamera_c::m_instance->mScreen);
    m3d::drawLightMapTexture(0);
    m3d::calcWorld(0);
    m3d::calcView(0);
    GXSetAlphaUpdate(false);
    m3d::drawOpa();
    m3d::drawXlu();
    m3d::drawDone(0);
    m2d::defaultSet();
    d2d::drawBefore();
    EffectManager_c::draw(0, 3);
    EffectManager_c::draw(0, 2);
    GXDrawDone();
    m3d::clear();
    m3d::reset();
    m3d::setCurrentCamera(1);
    dBaseActor_c::draw2DActorOnLyt1();
    m3d::drawLightMapTexture(1);
    m3d::calcWorld(1);
    m3d::calcView(1);
    m3d::calcMaterial();
    m3d::drawOpa();
    m3d::drawXlu();
    m3d::drawDone(1);
    m3d::setCurrentCamera(0);
    if (dInfo_c::m_instance->mDrawEffectsForMapLayouts) {
        for (int i = 0; i < 4; i++) {
            EffectManager_c::draw(0, 11+i);
            EffectManager_c::draw(0, 7+i);
        }
    }
    GXDrawDone();
    m2d::drawAfter(0x80);
    m2d::reset();
    m3d::setCurrentCamera(0);
}

// Behaves more like retail wm drawfunc
/*void KoopatlasDrawFunc() {
    int camID = m3d::getCurrentCameraID();
    m3d::screenEffectReset(camID, dKPCamera_c::m_instance->mScreen);
    m3d::drawLightMapTexture(0);
    m3d::calcWorld(0);
    m3d::calcView(0);
    EGG::StateGX::GXSetAlphaUpdate_(false);
    m3d::drawOpa();
    m3d::drawXlu();
    m3d::drawDone(0);
    // Screen clipper here?
    m2d::defaultSet();
    d2d::drawBefore();
    EffectManager_c::draw(0, 3);
    EffectManager_c::draw(0, 2);
    if (dInfo_c::m_instance->mIsWorldSelect != 0) {
        GXDrawDone();
        m3d::clear();
        m3d::reset();
        m3d::setCurrentCamera(1);
        dBaseActor_c::draw2DActorOnLyt1();
        m3d::drawLightMapTexture(1);
        m3d::calcWorld(1);
        m3d::calcView(1);
        m3d::calcMaterial();
        m3d::drawOpa();
        m3d::drawXlu();
        m3d::drawDone(1);
        GXDrawDone();
        m3d::clear();
        m3d::reset();
        GXSetZMode(false, GX_ALWAYS, false);
        d2d::drawBtween(0x80, 0x92);
        m3d::setCurrentCamera(1);
        dBaseActor_c::draw2DActorOnLyt2();
        m3d::drawLightMapTexture(1);
        m3d::calcWorld(1);
        m3d::calcView(1);
        m3d::calcMaterial();
        m3d::drawOpa();
        m3d::drawXlu();
        m3d::drawDone(1);
        if (dInfo_c::m_instance->mDrawEffectsForMapLayouts) {
            for (int i = 0; i < 4; i++) {
                EffectManager_c::draw(0, 0xB + i);
                EffectManager_c::draw(0, 7 + i);
            }
        }
        GXDrawDone();
        m3d::clear();
        m3d::reset();
        d2d::drawBtween(0x92, 0xFF);
        m2d::reset();
        m3d::setCurrentCamera(0);
        // Screen clipper here too?
    }
}*/

#endif