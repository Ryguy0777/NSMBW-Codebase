#include <new/game_config.h>

#ifdef KOOPATLAS_DEV_ENABLED
#include <kamek.h>

#include <new/bases/koopatlas/d_s_koopatlas.hpp>
#include <new/bases/koopatlas/d_kp_music.hpp>
#include <new/bases/koopatlas/d_kp_camera.hpp>
#include <new/bases/koopatlas/d_a_kp_player.hpp>
// #include "music.h"

#include <constants/sound_list.h>
#include <game/bases/d_3d.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_base_actor.hpp>
#include <game/bases/d_course_select_manager.hpp>
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
#include <game/bases/d_effectmanager.hpp>
#include <game/bases/d_wipe_circle.hpp>
#include <game/framework/f_profile.hpp>
#include <game/mLib/m_pad.h>
#include <game/snd/snd_scene_manager.hpp>

// remove me later
#include <game/bases/d_system.hpp>

// Reset "LastPowerupStoreType" after game over
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

dScKoopatlas_c *dScKoopatlas_c::instance = nullptr;

STATE_DEFINE(dScKoopatlas_c, Limbo);
STATE_DEFINE(dScKoopatlas_c, ContinueWait);
STATE_DEFINE(dScKoopatlas_c, Normal);
STATE_DEFINE(dScKoopatlas_c, CompletionMsg);
STATE_DEFINE(dScKoopatlas_c, CompletionMsgHideWait);
STATE_DEFINE(dScKoopatlas_c, CSMenu);
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

dScKoopatlas_c *dScKoopatlas_c::build() {
    MapReport("Creating WorldMap\n");
    dScKoopatlas_c *c = new dScKoopatlas_c;

    MapReport("Created WorldMap @ %p\n", c);
    instance = c;
    return c;
}

// Replace WORLD_MAP actor
kmWritePointer(0x8098DCF0, &dScKoopatlas_c::build);

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

sPhase_c::phaseMethod *initFunctions[] = {
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

dScKoopatlas_c::dScKoopatlas_c() : mInitChain(initFunctions, 9), mStateMgr(*this, StateID_Normal) {
    mpPhase = &mInitChain;
}

dScKoopatlas_c::~dScKoopatlas_c() {
    instance = nullptr;
}



sPhase_c::METHOD_RESULT_e KPInitPhase_LoadSceneSnd(void *ptr) {
    SpammyReport("KPInitPhase_LoadSceneSnd called\n");

    dAudio::loadSceneSnd();
    return (sPhase_c::METHOD_RESULT_e)true;
}

sPhase_c::METHOD_RESULT_e KPInitPhase_LoadKPMusic(void *ptr) {
    SpammyReport("KPInitPhase_LoadKPMusic called\n");
    //return (sPhase_c::METHOD_RESULT_e)dKPMusic_c::loadInfo();
    return (sPhase_c::METHOD_RESULT_e)true;
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

    if (dResMng_c::m_instance->mRes.syncAllRes()) {
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

    //return (sPhase_c::METHOD_RESULT_e)(wm->mMapListLoader.request("/Maps/List.txt", 0, nullptr) != nullptr);
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
        OSReport("Load map: %s\n", wm->mMapPath);
    }*/

    /*if (wm->mMapData.load(wm->mMapPath)) {
        return (sPhase_c::METHOD_RESULT_e)true;
    } else
        return (sPhase_c::METHOD_RESULT_e)false;*/
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
        SpammyReport("continue is activated\n");
        wm->mStateMgr.changeState(wm->StateID_ContinueWait);
    } else {
        SpammyReport("continue is not activated\n");
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

    // need Player before we can set up paths
    SpammyReport("creating player\n");
    wm->mpPlayer = (daKPPlayer_c*)fBase_c::createChild(fProfile::WM_PLAYER, wm, 0, 2);
    wm->mpPlayer->mpPyMdlMng->mpMdl->setPlayerMode(daPyMng_c::mPlayerMode[0]);
    wm->mpPlayer->bindPats();
    wm->mpPlayer->mpPyMdlMng->mpMdl->setAnm(0, 1.2f, 10.0f, 0.0f);

    // since we've got all the resources, set up the path data too
    /*SpammyReport("preparing path manager\n");
    wm->mPathManager.setup();*/

    // and put the player into position
    //dKPNode_s *cNode = wm->mPathManager.currentNode;
    //wm->mpPlayer->mPos = mVec3_c(cNode->x, -cNode->y, wm->mpPlayer->mPos.z);

    wm->mpPlayer->mPos = mVec3_c(250.0f, 100.0f, wm->mpPlayer->mPos.z);

    /*SpammyReport("creating MAP\n");
    wm->mpMap = (dWMMap_c*)fBase_c::createChild(fProfile::WM_MAP, wm, 0, 0);*/

    SpammyReport("creating HUD\n");
    wm->mpHud = (dWMHud_c*)fBase_c::createChild(fProfile::WM_HUD, wm, 0, 0);

    /*SpammyReport("creating SHOP\n");
    wm->mpShop = (dWMShop_c*)fBase_c::createChild(fProfile::WM_SHOP, wm, 0, 2);*/

    SpammyReport("creating Star Coin Menu\n");
    wm->mpCoins = (dWMStarCoin_c*)fBase_c::createChild(fProfile::WM_STARCOIN, wm, 0, 0);

    SpammyReport("SetupExtra done\n");
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

    SpammyReport("KPInitPhase_ChkChildProcess returning true\n");
    return (sPhase_c::METHOD_RESULT_e)true;
}



// todo: figure out what i wanna do with savedata
void dScKoopatlas_c::startMusic() {
    //dKPMusic_c::play(GetSaveFile()->GetBlock(-1)->currentMapMusic);
    //dKPMusic_c::play(2);
}

void dScKoopatlas_c::setupScene() {
    d3d::createLightMgr(EGG::Heap::sCurrentHeap, 36, 8, 2, false, 0);
    loadScene(0);
    d3d::createLightMgr(EGG::Heap::sCurrentHeap, 36, 8, 2, true, 1);
    loadScene(1);
}

void dScKoopatlas_c::loadScene(int sceneID) {
    EGG::LightManager *lightMgr = m3d::getLightMgr(sceneID);

    nw4r::g3d::ResFile res = dResMng_c::m_instance->mRes.getRes("Env_world", "scene/scene.brres");
    nw4r::g3d::ResAnmScn anmScn = res.GetResAnmScn("MainSelect");
    anmScn.Bind(anmScn);

    lightMgr->LoadScnLightInner(anmScn, 0.0f, -1, -3);

    if (sceneID == 0) { // Main lighting
        nw4r::g3d::ResFile blight = dResMng_c::m_instance->mRes.getRes("Env_world", "light/W8.blight");
        lightMgr->LoadBlight(&blight);

        nw4r::g3d::ResFile blmap = dResMng_c::m_instance->mRes.getRes("Env_world", "light/W8.blmap");
        lightMgr->ltMgr->LoadBlmap(&blmap);

    } else if (sceneID == 1) { // Layout model lighting
        nw4r::g3d::ResFile blight = dResMng_c::m_instance->mRes.getRes("Env_world", "light/Layout3D.blight");
        lightMgr->LoadBlight(&blight);

        nw4r::g3d::ResFile blmap = dResMng_c::m_instance->mRes.getRes("Env_world", "light/Layout3D.blmap");
        lightMgr->ltMgr->LoadBlmap(&blmap);

        EGG::FogManager *fogMgr = m3d::getFogMgr(sceneID);
        nw4r::g3d::ResFile bfog = dResMng_c::m_instance->mRes.getRes("Env_world", "light/Layout3D.bfog");
        fogMgr->LoadBfog(&bfog);
    }
}

int dScKoopatlas_c::create() {
    OSReport("KP scene param: %08x\n", mParam);
    SpammyReport("create() called\n");

    dFader_c::setFader(dFader_c::CIRCLE_SLOW);

    // temp
    nw4r::ut::Color clr(16, 58, 135, 255);
    dSys_c::setClearColor(clr);

    SpammyReport("Freeing effects\n"); // Opening cutscene loads vs effects for some reason and fragments RAM too much for some maps
    EffectManager_c::resetResource(EffectManager_c::EFF_VS);
    EffectManager_c::freeBreff(EffectManager_c::EFF_VS);
    EffectManager_c::freeBreft(EffectManager_c::EFF_VS);

    SpammyReport("Preparing lighting\n");
    setupScene();

    SpammyReport("Setting Active Players\n");
    for (int i = 0; i < 4; i++) {
        bool isThere = dGameCom::PlayerEnterCheck(i);
        int id = daPyMng_c::mPlayerType[i];
        daPyMng_c::mPlayerEntry[i] = isThere ? 1 : 0;
        if (!isThere) daPyMng_c::mCreateItem[i] = 0;
    }

    SpammyReport("select cursor\n");
    this->mpSelectCursor = (dSelectCursor_c*)fBase_c::createChild(fProfile::SELECT_CURSOR, this, 0, 0);

    SpammyReport("cs menu\n");
    this->mpCourseSelectMenu = (dCourseSelectMenu_c*)fBase_c::createChild(fProfile::COURSE_SELECT_MENU, this, 0, 0);

    SpammyReport("yes no window\n");
    this->mpYesNoWindow = (dYesNoWindow_c*)fBase_c::createChild(fProfile::YES_NO_WINDOW, this, 0, 0);

    SpammyReport("number of people change\n");
    this->mpNumPeopleChange = (dNumberOfPeopleChange_c*)fBase_c::createChild(fProfile::NUMBER_OF_PEOPLE_CHANGE, this, 0, 0);

    for (int i = 0; i < 4; i++) {
        SpammyReport("ccsb %d\n", i);
        dCharacterChangeSelectBase_c *ccsb = (dCharacterChangeSelectBase_c*)fBase_c::createChild(fProfile::CHARACTER_CHANGE_SELECT_BASE, this, i, 0);

        SpammyReport("ccsc %d\n", i);
        dCharacterChangeSelectContents_c *ccsc = (dCharacterChangeSelectContents_c*)fBase_c::createChild(fProfile::CHARACTER_CHANGE_SELECT_CONTENTS, this, i, 0);

        SpammyReport("ccsa %d\n", i);
        void *ccsa = fBase_c::createChild(fProfile::CHARACTER_CHANGE_SELECT_ARROW, this, i, 0);

        SpammyReport("cci %d\n", i);
        void *cci = fBase_c::createChild(fProfile::CHARACTER_CHANGE_INDICATOR, this, i, 0);

        mpNumPeopleChange->mpSelectBases[i] = ccsb;
        mpNumPeopleChange->mpSelectContents[i] = ccsc;
        mpNumPeopleChange->mpSelectArrows[i] = ccsa;
        mpNumPeopleChange->mpIndicators[i] = cci;
    }

    SpammyReport("continue\n");
    this->mpContinue = (dContinue_c*)fBase_c::createChild(fProfile::CONTINUE, this, 0, 0);

    SpammyReport("stock item\n");
    this->mpStockItem = (dStockItem_c*)fBase_c::createChild(fProfile::STOCK_ITEM, this, 0, 0);
    SpammyReport("stock item shadow\n");
    this->mpStockItemShadow = (dStockItemShadow_c*)fBase_c::createChild(fProfile::STOCK_ITEM_SHADOW, this, 0, 0);
    mpStockItem->mpShadow = mpStockItemShadow;

    SpammyReport("easy pairing\n");
    this->mpEasyPairing = (dEasyPairing_c*)fBase_c::createChild(fProfile::EASY_PAIRING, this, 0, 0);

    SpammyReport("world camera\n");
    fBase_c::createChild(fProfile::WORLD_CAMERA, this, 0, 0);

    SpammyReport("setting NewerMapDrawFunc\n");
    dGraph_c::ms_Instance->mpDrawFunc = NewerMapDrawFunc;

    SpammyReport("create() completed\n");
    
    // Prepare this first
    dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
    mCurrentMapID = save->getCurrentWorld();
    mIsFirstPlay = (mCurrentMapID == 0) && (mParam & 0x80000000);

    // Are we coming from Kamek cutscene? If so, then do.. some stuff!
    mIsAfterKamekCutscene = (mParam & 0x40000000);
    if (mIsAfterKamekCutscene) {
        mCurrentMapID = 6; // KoopaPlanet
        save->setCurrentWorld(6);
    }

    /*if (MaybeFinishingLevel[0] == 7 && MaybeFinishingLevel[1] == 24 && save->isCourseDataFlag(7, 24, dMj2dGame_c::GOAL_NORMAL)) {
        mCurrentMapID = 7; // KoopaPlanetUnd
        save->setCurrentWorld(7);
        mIsAfter8Castle = true;
    }*/

    mIsEndingScene = (mParam & 0x20000000);
    if (mIsEndingScene) {
        mCurrentMapID = 0;
        save->setCurrentWorld(0);
        save->setCurrentPathNode(0);
    }

    SndSceneMgr::sInstance->closeWorldSelect();
    return true;
}

int dScKoopatlas_c::doDelete() {
    //if (!mKeepMusicPlaying)
    //	dKPMusic_c::stop();

    m3d::removeLightMgr(0);
    m3d::removeLightMgr(1);

    EffectManager_c::courseOut();
    EffectManager_c::resetResource(EffectManager_c::EFF_MAP);
    EffectManager_c::freeBreff(EffectManager_c::EFF_MAP);
    EffectManager_c::freeBreft(EffectManager_c::EFF_MAP);

    dResMng_c::m_instance->mRes.deleteRes("SI_kinoko");
    dResMng_c::m_instance->mRes.deleteRes("SI_fireflower");
    dResMng_c::m_instance->mRes.deleteRes("SI_iceflower");
    dResMng_c::m_instance->mRes.deleteRes("SI_penguin");
    dResMng_c::m_instance->mRes.deleteRes("SI_propeller");
    dResMng_c::m_instance->mRes.deleteRes("SI_star");

    dResMng_c::m_instance->mRes.deleteRes("cobCourse");
    dResMng_c::m_instance->mRes.deleteRes("I_kinoko_bundle");
    dResMng_c::m_instance->mRes.deleteRes("lakitu");
    dResMng_c::m_instance->mRes.deleteRes("star_coin");
    dResMng_c::m_instance->mRes.deleteRes("StarRing");

    mMapListLoader.freeResouce();

    EffectManager_c::courseOut();
    return true;
}

bool dScKoopatlas_c::canDoStuff() {
    if (dGameCom::isGameStop(0xFFFFFFFF)) return false;
    if (IsWarningManagerActive()) return false;
    if (mStateMgr.getStateID() == &StateID_Limbo) return false;
    return true;
}

bool dScKoopatlas_c::mapIsRunning() {
    if (dGameCom::isGameStop(0xFFFFFFFF)) return false;
    if (IsWarningManagerActive()) return false;
    if (mStateMgr.getStateID() != &StateID_Normal) return false;
    return true;
}


int dScKoopatlas_c::execute() {
    //dKPMusic_c::execute();
    if (!canDoStuff()) return true;

    //SpammyReport("Executing mStateMgr: %s\n", mStateMgr.getStateID()->name());
    mStateMgr.executeState();
    return true;
}


void dScKoopatlas_c::initializeState_Limbo() { }
void dScKoopatlas_c::executeState_Limbo() { }
void dScKoopatlas_c::finalizeState_Limbo() { }


void dScKoopatlas_c::initializeState_ContinueWait() {
    dInfo_c::m_instance->mDrawEffectsForMapLayouts = true;
    mpContinue->mIsVisible = true;
    mpContinue->mWillOpen = true;
    mpContinue->mIsGameOver = false;
}
void dScKoopatlas_c::executeState_ContinueWait() {
    // Waiting for the Continue anim to finish
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


void dScKoopatlas_c::initializeState_Normal() { }
void dScKoopatlas_c::executeState_Normal() {
    /*if (mPathManager.completionMessagePending) {
        OSReport("Going to set CompletionMsg\n");
        mStateMgr.changeState(StateID_CompletionMsg);
        return;
    }

    if (mPathManager.doingThings())
        return;*/

    int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;

    // Nothing related to the menu is going on
    if (pressed & WPAD_BUTTON_1) {
        SndAudioMgr::sInstance->startSystemSe(SE_SYS_DECIDE, 1);
        mpStockItem->mIsVisible = true;
        mStateMgr.changeState(StateID_PowerupsWait);
        mpHud->hideAll();
    } else if (pressed & WPAD_BUTTON_PLUS) {
        SndAudioMgr::sInstance->startSystemSe(SE_SYS_PAUSE, 1);
        mpCourseSelectMenu->mIsVisible = true;
        mStateMgr.changeState(StateID_CSMenu);
        mpHud->hideAll();
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
    }
}
void dScKoopatlas_c::finalizeState_Normal() { }

void dScKoopatlas_c::initializeState_CSMenu() { }
void dScKoopatlas_c::executeState_CSMenu() {
    // The course select menu is currently being shown

    // First off, check to see if it's been hidden
    if (!mpCourseSelectMenu->mIsVisible) {
        // That means something happened
        if (mpCourseSelectMenu->mChoiceMade) {
            // Player pressed a button
            dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);

            switch (mpCourseSelectMenu->mCurrentSelection) {
                case 0: // Star Coins
                    mpCoins->show();
                    mStateMgr.changeState(StateID_CoinsWait);
                    break;

                case 1: // Add/Drop Players
                    MapReport("Add/Drop Players was pressed\n");
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
            // Ok, change back to STATE_Normal
            mpHud->unhideAll();
            mStateMgr.changeState(StateID_Normal);
        }
    }
}
void dScKoopatlas_c::finalizeState_CSMenu() { }

void dScKoopatlas_c::initializeState_TitleConfirmOpenWait() { }
void dScKoopatlas_c::executeState_TitleConfirmOpenWait() {
    // Waiting for the "Go to Title Screen" YesNoWindow to finish opening
    if (!mpYesNoWindow->mIsAnimating) {
        mStateMgr.changeState(StateID_TitleConfirmSelect);
    }
}
void dScKoopatlas_c::finalizeState_TitleConfirmOpenWait() { }

/**********************************************************************/
// STATE_TitleConfirmSelect : Let the user choose an option on the
// 	"Go to Title Screen" YesNoWindow.
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
// STATE_TitleConfirmHitWait : Process the user's chosen option on
// 	the "Go to Title Screen" YesNoWindow. Also, wait for the
//  animation to be complete.
void dScKoopatlas_c::initializeState_TitleConfirmHitWait() { }
void dScKoopatlas_c::executeState_TitleConfirmHitWait() {
    if (!mpYesNoWindow->mIsAnimating) {
        if (mpYesNoWindow->getCursorPos() == 1) {
            mStateMgr.changeState(StateID_Normal);
            mpHud->unhideAll();
        } else {
            mStateMgr.changeState(StateID_Limbo);
            dScRestartCrsin_c::startTitle(false, 0);
        }
    }
}
void dScKoopatlas_c::finalizeState_TitleConfirmHitWait() { }

/**********************************************************************/
// STATE_PlayerChangeWait : Wait for the user to do something on the
// 	Add/Drop Players screen.
void dScKoopatlas_c::initializeState_PlayerChangeWait() { }
void dScKoopatlas_c::executeState_PlayerChangeWait() {
    int nowPressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;

    if (mpNumPeopleChange->mAllowEasyPairing) {
        if (nowPressed & WPAD_BUTTON_PLUS) {
            // activate easy pairing. FUN !!
            mpNumPeopleChange->mIsEasyPairingActive = true;

            for (int i = 0; i < 4; i++) {
                mpNumPeopleChange->mpPlayers[i]->mIsEasyPairingActive = true;
                mpNumPeopleChange->mpSelectBases[i]->mIsEasyPairingActive = true;
                mpNumPeopleChange->mpSelectContents[i]->mIsEasyPairingActive = true;
            }

            mpEasyPairing->mIsVisible = true;
            mStateMgr.changeState(StateID_EasyPairingWait);
        }
    } else {
        if (!mpNumPeopleChange->mIsVisible) {
            for (int i = 0; i < 4; i++) {
                bool isThere = dGameCom::PlayerEnterCheck(i);
                int id = daPyMng_c::mPlayerType[i];
                daPyMng_c::mPlayerEntry[i] = isThere ? 1 : 0;
                if (!isThere) daPyMng_c::mCreateItem[i] = 0;
            }

            mStateMgr.changeState(StateID_Normal);
            mpHud->unhideAll();
        }
    }

}
void dScKoopatlas_c::finalizeState_PlayerChangeWait() { }

/**********************************************************************/
// STATE_EasyPairingWait : Wait for the user to exit Easy Pairing.
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
// STATE_PowerupsWait : Wait for the user to exit the Powerups screen.
void dScKoopatlas_c::initializeState_PowerupsWait() { }
void dScKoopatlas_c::executeState_PowerupsWait() {
    if (!mpStockItem->mIsVisible) {
        //mpPlayer->modelHandler->mdlClass->setPowerup(daPyMng_c::mPlayerType[0]);
        //mpPlayer->bindPats();

        mStateMgr.changeState(StateID_Normal);
        mpHud->unhideAll();
    }
}
void dScKoopatlas_c::finalizeState_PowerupsWait() { }


/**********************************************************************/
// STATE_ShopWait : Wait for the user to exit the Shop screen.
void dScKoopatlas_c::initializeState_ShopWait() { }
void dScKoopatlas_c::executeState_ShopWait() {
    //if (!mpShop->visible) {
    //	mStateMgr.changeState(StateID_Normal);
    //	//mpHud->unhideAll();
    //}
}
void dScKoopatlas_c::finalizeState_ShopWait() { }


/**********************************************************************/
// STATE_StarCoin : Wait for the user to exit the Star Coin screen.
void dScKoopatlas_c::initializeState_CoinsWait() { }
void dScKoopatlas_c::executeState_CoinsWait() {
    if (!mpCoins->mIsVisible) {
    	mStateMgr.changeState(StateID_Normal);
    	mpHud->unhideAll();
    }
}
void dScKoopatlas_c::finalizeState_CoinsWait() { }

/**********************************************************************/
// STATE_SaveOpen : Waiting for the "Save?" YesNoWindow to open
void dScKoopatlas_c::initializeState_SaveOpen() { }
void dScKoopatlas_c::finalizeState_SaveOpen() { }
void dScKoopatlas_c::executeState_SaveOpen() {
    if (!mpYesNoWindow->mIsAnimating) {
        mStateMgr.changeState(StateID_SaveSelect);
    }
}

/**********************************************************************/
// STATE_SaveSelect : Let the user choose an option on the
// 	"Save?" YesNoWindow.
void dScKoopatlas_c::initializeState_SaveSelect() { }
void dScKoopatlas_c::finalizeState_SaveSelect() { }
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

/**********************************************************************/
// STATE_SaveWindowClose : Process the user's chosen option on the
// 	"Save?" YesNoWindow. Also, wait for the animation to be complete.
void dScKoopatlas_c::initializeState_SaveWindowClose() { }
void dScKoopatlas_c::finalizeState_SaveWindowClose() { }
void dScKoopatlas_c::executeState_SaveWindowClose() {
    if (!mpYesNoWindow->getIsActive()) {
        if (mpYesNoWindow->getCursorPos() == 1) {
            mStateMgr.changeState(StateID_Normal);
            mpHud->unhideAll();
        } else {
            mStateMgr.changeState(StateID_SaveDo);
            dCourseSelectManager_c::m_instance->saveGame(false);
        }
    }
}

/**********************************************************************/
// STATE_SaveDo : Save the game.
void dScKoopatlas_c::initializeState_SaveDo() { }
void dScKoopatlas_c::finalizeState_SaveDo() { }
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

/**********************************************************************/
// STATE_SaveEndWindow : Handle the Save End window.
void dScKoopatlas_c::initializeState_SaveEndWindow() { }
void dScKoopatlas_c::finalizeState_SaveEndWindow() { }
void dScKoopatlas_c::executeState_SaveEndWindow() {
    if (!mpYesNoWindow->mIsAnimating) {
        int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;
        if (pressed & (WPAD_BUTTON_A | WPAD_BUTTON_2)) {
            mpYesNoWindow->mHitButton = true;
            mStateMgr.changeState(StateID_SaveEndCloseWait);
        }
    }
}

/**********************************************************************/
// STATE_SaveEndCloseWait : Wait for the Save End window to close.
void dScKoopatlas_c::initializeState_SaveEndCloseWait() { }
void dScKoopatlas_c::finalizeState_SaveEndCloseWait() { }
void dScKoopatlas_c::executeState_SaveEndCloseWait() {
    if (!mpYesNoWindow->mIsAnimating) {
        mStateMgr.changeState(StateID_Normal);
        mpHud->unhideAll();
    }
}

#ifndef DISABLE_QUICK_SAVE
/**********************************************************************/
// STATE_QuickSaveOpen : Waiting for the "Save?" YesNoWindow to open
void dScKoopatlas_c::initializeState_QuickSaveOpen() { }
void dScKoopatlas_c::finalizeState_QuickSaveOpen() { }
void dScKoopatlas_c::executeState_QuickSaveOpen() {
    if (!mpYesNoWindow->mIsAnimating) {
        mStateMgr.changeState(StateID_QuickSaveSelect);
    }
}

/**********************************************************************/
// STATE_QuickSaveSelect : Let the user choose an option on the
// 	"Save?" YesNoWindow.
void dScKoopatlas_c::initializeState_QuickSaveSelect() { }
void dScKoopatlas_c::finalizeState_QuickSaveSelect() { }
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

/**********************************************************************/
// STATE_QuickSaveWindowClose : Process the user's chosen option on
// 	the "Save?" YesNoWindow. Also, wait for the animation to be complete
void dScKoopatlas_c::initializeState_QuickSaveWindowClose() { }
void dScKoopatlas_c::finalizeState_QuickSaveWindowClose() { }
void dScKoopatlas_c::executeState_QuickSaveWindowClose() {
    if (!mpYesNoWindow->getIsActive()) {
        if (mpYesNoWindow->getCursorPos() == 1) {
            mStateMgr.changeState(StateID_Normal);
            mpHud->unhideAll();
        } else {
            mStateMgr.changeState(StateID_QuickSaveDo);
            dCourseSelectManager_c::m_instance->saveGame(true);
        }
    }
}

/**********************************************************************/
// STATE_QuickSaveDo : Save the game.
void dScKoopatlas_c::initializeState_QuickSaveDo() { }
void dScKoopatlas_c::finalizeState_QuickSaveDo() { }
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

// STATE_QuickSaveEndWindow : Handle the Save End window.
void dScKoopatlas_c::initializeState_QuickSaveEndWindow() { }
void dScKoopatlas_c::finalizeState_QuickSaveEndWindow() { }
void dScKoopatlas_c::executeState_QuickSaveEndWindow() {
    if (!mpYesNoWindow->mIsAnimating) {
        int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;
        if (pressed & (WPAD_BUTTON_A | WPAD_BUTTON_2)) {
            mpYesNoWindow->mHitButton = true;
            mStateMgr.changeState(StateID_QuickSaveEndCloseWait);
        }
    }
}

// STATE_QuickSaveEndCloseWait : Wait for Save End window to close
void dScKoopatlas_c::initializeState_QuickSaveEndCloseWait() { }
void dScKoopatlas_c::finalizeState_QuickSaveEndCloseWait() { }
void dScKoopatlas_c::executeState_QuickSaveEndCloseWait() {
    if (!mpYesNoWindow->mIsAnimating) {
        if (mpYesNoWindow->getCursorPos() == 1) {
            mStateMgr.changeState(StateID_Normal);
            mpHud->unhideAll();
        } else {
            mStateMgr.changeState(StateID_Limbo);
            dScRestartCrsin_c::startTitle(false, 0);
        }
    }
}
#endif

void dScKoopatlas_c::initializeState_SaveError() { }
void dScKoopatlas_c::finalizeState_SaveError() { }
void dScKoopatlas_c::executeState_SaveError() {
    // TODO: Me!
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


void dScKoopatlas_c::showSaveWindow() {
    mpHud->hideAll();
    mStateMgr.changeState(StateID_SaveOpen);
    mpYesNoWindow->setType(dYesNoWindow_c::SAVE);
    mpYesNoWindow->setIsActive(true);
}

// added to bmg
static const wchar_t *completionMsgs[] = {
    L"The most erudite of Buttocks",
    L"You've collected all of\nthe \x0B\x014F\xBEEF Star Coins in\n",
    L"You have gotten every \x0B\x013B\xBEEF exit\nin",
    L"You have gotten everything\nin",
    L"You have collected all the\nnecessary \x0B\x014F\xBEEF coins to enter\nthe Special World!",
    L"You have collected all the \x0B\x014F\xBEEF Star\nCoins in the game!",
    L"You've found every \x0B\x013B\xBEEF exit in the\ngame!",
    L"You've completed everything in\nNEWER SUPER MARIO BROS. Wii!\n\nWe present to you a new quest.\nTry pressing \x0B\x0122\xBEEF and \x0B\x0125\xBEEF\n on the Star Coin menu."
};

void dScKoopatlas_c::initializeState_CompletionMsg() {
    //OSReport("CompletionMsg beginning with type %d\n", mPathManager.completionMessageType);
    static const int ynTypes[8] = {
        /*NULL*/ -1,
        /*COINS*/ 14,
        /*EXITS*/ 7,
        /*WORLD*/ 8,
        /*COINS EXC W9*/ 9,
        /*GLOBAL COINS*/ 11,
        /*GLOBAL EXITS*/ 27,
        /*EVERYTHING*/ 21
    };
    //mpYesNoWindow->setType(ynTypes[mPathManager.completionMessageType]);
    mpYesNoWindow->setIsActive(true);
    mMustFixYesNoText = 10; // hacky shit
}

void dScKoopatlas_c::finalizeState_CompletionMsg() {
    //mPathManager.completionMessagePending = false;
    //mPathManager.completionMessageType = 0;
}

void dScKoopatlas_c::executeState_CompletionMsg() {
    // hacky shit
    /*if (mMustFixYesNoText > 0) {
        mMustFixYesNoText--;

        int type = mPathManager.completionMessageType;

        const wchar_t *baseText = completionMsgs[type];
        // Used when we assemble a dynamic message
        wchar_t text[512];

        if (type >= CMP_MSG_COINS && type <= CMP_MSG_WORLD) {
            // title
            int w = mPathManager.completionMessageWorldNum;
            int l = ((w == 5) || (w == 7)) ? 101 : 100;
            dLevelInfo_c::entry_s *titleEntry = dLevelInfo_c::m_instance.getEntryFromDispID(w, l);
            const char *title = dLevelInfo_c::m_instance.getLevelName(titleEntry);

            // assemble the string

            wcscpy(text, baseText);
            int pos = wcslen(text);

            text[pos++] = ' ';

            while (*title) {
                char chr = *(title++);
                if (chr != '-')
                    text[pos++] = chr;
            }

            text[pos++] = '!';
            text[pos++] = 0;
            baseText = text;
        }

        LytTextBox_c *T_questionS_00 = mpYesNoWindow->getTextBox(0);
        LytTextBox_c *T_question_00 = mpYesNoWindow->getTextBox(1);
        T_question_00->SetString(baseText, 0);
        T_questionS_00->SetString(baseText, 0);
    }

    if (!mpYesNoWindow->mIsAnimating) {
        int pressed = dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons;
        if (pressed & (WPAD_BUTTON_A | WPAD_BUTTON_2)) {
            mpYesNoWindow->mHitButton = true;
            mStateMgr.changeState(StateID_CompletionMsgHideWait);
        }
    }*/
}

void dScKoopatlas_c::initializeState_CompletionMsgHideWait() { }
void dScKoopatlas_c::finalizeState_CompletionMsgHideWait() { }
void dScKoopatlas_c::executeState_CompletionMsgHideWait() {
    if (!mpYesNoWindow->getIsActive())
        mStateMgr.changeState(StateID_Normal);
}

void NewerMapDrawFunc() {
    m3d::reset();
    m3d::setCurrentCamera(0);
    m3d::screenEffectReset(0, dKPCamera_c::m_instance->mScreen);
    m3d::drawLightMapTexture(0);
    m3d::calcWorld(0);
    m3d::calcView(0);
    GXSetAlphaUpdate(false);
    m3d::drawOpa();
    m3d::drawXlu();
    m3d::drawDone(0);
    m2d::defaultSet();
    m2d::drawBefore(0x81);
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
    for (int i = 0; i < 4; i++) {
        EffectManager_c::draw(0, 11+i);
        EffectManager_c::draw(0, 7+i);
    }
    GXDrawDone();
    m2d::drawAfter(0x80);
    m2d::reset();
    m3d::setCurrentCamera(0);
}

#endif