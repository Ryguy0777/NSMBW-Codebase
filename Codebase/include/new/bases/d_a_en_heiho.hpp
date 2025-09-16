#pragma once

#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_audio.hpp>
#include <constants/sound_list.h>

// custom shyguy enemy

class daEnHeiho_c : public dEn_c {
public:
    enum HEIHO_TYPE_e {
        HEIHO_TYPE_WALKER,
        HEIHO_TYPE_WALKER_LEDGE,
        HEIHO_TYPE_SLEEPER,
        HEIHO_TYPE_JUMPER,
        HEIHO_TYPE_PACER,
    };
    
    daEnHeiho_c() : mRes(nullptr) {};
    ~daEnHeiho_c() {};

    virtual int create();
    virtual int execute();
    virtual int draw();

    virtual void finalUpdate();

    virtual void Normal_VsEnHitCheck(dCc_c *cc1, dCc_c *cc2);
    virtual void Normal_VsPlHitCheck(dCc_c *cc1, dCc_c *cc2);
    virtual void Normal_VsYoshiHitCheck(dCc_c *cc1, dCc_c *cc2);

    virtual void initalizeState_DieFall();
    virtual void executeState_DieFall();

    virtual void initializeState_DieOther();
    virtual void executeState_DieOther();

    virtual void createIceActor();

    void setWalkSpeed();
    void playChrAnim(const char* name, m3d::playMode_e playMode, float blendFrame, float rate);
    void updateModel();
    bool checkForLedge(float xOffset);

    void reactFumiProc(dActor_c *player);
    void reactSpinFumiProc(dActor_c *player);
    void reactYoshiFumiProc(dActor_c *player);

    STATE_FUNC_DECLARE(daEnHeiho_c, Walk);
	STATE_FUNC_DECLARE(daEnHeiho_c, Turn);
    STATE_FUNC_DECLARE(daEnHeiho_c, Sleep);
    STATE_FUNC_DECLARE(daEnHeiho_c, Jump);
    STATE_FUNC_DECLARE(daEnHeiho_c, Dizzy);
    STATE_FUNC_DECLARE(daEnHeiho_c, Idle);

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mRes;
    m3d::mdl_c mHeihoModel;
    m3d::anmChr_c mAnmChr;
    nw4r::g3d::ResAnmTexPat mResPat;
    m3d::anmTexPat_c mAnmTexPat;

    int mTimer;
	float mDying;
	float mBaseline;
    float mFinalPos[2];
    int mJumpCounter;

    //mEf::levelEffect_c mDizzyEffect;
    sStateIDIf_c *mRecoverState;

    HEIHO_TYPE_e mType;
    int mColor;
    u8 mHealth;
    int mDistance;
    u8 mSpawnDir;
};

const s16 l_look_angle[2] = {0x2000, 0xE000};
const float l_walk_speed[2] = {0.6, -0.6};