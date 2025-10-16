#pragma once

#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_res_mng.hpp>

class daEnPuchiPakkun_c : public dEn_c {
public:
    // fBase_c overrides
    virtual int create();
    virtual int execute();
    virtual int draw();

    // dBaseActor_c overrides
    virtual void finalUpdate();

    // dEn_c overrides
    virtual void Normal_VsEnHitCheck(dCc_c *cc1, dCc_c *cc2);

    virtual void initializeState_Ice();
    virtual void finalizeState_Ice();

    virtual void createIceActor();

    void playChrAnim(const char* name, m3d::playMode_e playMode, float blendFrame, float rate);
    void updateModel();
    void setWalkSpeed();
    bool checkForLedge(float xOffset);
    bool isPlayerAbove();
    bool isPlayerInFireRange();
    void setFireDistance(float distance);

    STATE_FUNC_DECLARE(daEnPuchiPakkun_c, Idle);
    STATE_FUNC_DECLARE(daEnPuchiPakkun_c, Walk);
	STATE_FUNC_DECLARE(daEnPuchiPakkun_c, Turn);
    STATE_FUNC_DECLARE(daEnPuchiPakkun_c, Jump);
    STATE_FUNC_DECLARE(daEnPuchiPakkun_c, FireSpit);
    STATE_FUNC_DECLARE(daEnPuchiPakkun_c, IceWait);

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mRes;
    m3d::mdl_c mNipperModel;
    m3d::anmChr_c mAnmChr;

    float mStoredIcePos;
    bool mIsBahJump;
    u8 mFireDirection;
    u8 mFireDist;
    int mFireTimer;
    int mSpatFireCount;
    int mFireCooldown;

    bool mWalks;
    bool mSpitsFire;
    int mJumpHeight;
};

const s16 l_nipper_look_angle[2] = {0x4000, -0x4000};
const s16 l_nipper_turn_angle[2] = {0x3999, -0x3999};
const float l_nipper_walk_speed[2] = {0.5, -0.5};