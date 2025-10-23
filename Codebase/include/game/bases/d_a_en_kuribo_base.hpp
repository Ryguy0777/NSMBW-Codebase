#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_effect.hpp>

class daEnKuriboBase_c : public dEn_c {
public:
    // fBase_c overrides
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    virtual ~daEnKuriboBase_c();

    // dBaseActor_c overrides
    virtual void finalUpdate();

    // dEn_c overrides
    virtual bool EnDamageCheck(dCc_c *cc1, dCc_c *cc2);
    virtual bool PlDamageCheck(dCc_c *cc1, dCc_c *cc2);
    virtual bool YoshiDamageCheck(dCc_c *cc1, dCc_c *cc2);

    virtual void initializeState_DieOther();
    virtual void executeState_DieOther();
    virtual void finalizeState_DieOther();

    virtual void fumidamageSE(const mVec3_c &pos, int);
    virtual void setDamage(dActor_c *actor);

    virtual void beginFunsui();
    virtual void endFunsui();
    virtual void isFunsui() const;

    // New virtuals
    virtual void initializeState_Walk();
    virtual void executeState_Walk();
    virtual void finalizeState_Walk();
    virtual void initializeState_Turn();
    virtual void executeState_Turn();
    virtual void finalizeState_Turn();
    virtual void initializeState_TrplnJump();
    virtual void executeState_TrplnJump();
    virtual void finalizeState_TrplnJump();

    virtual void calcModel();
    virtual void reactFumiProc(dActor_c* player);
    virtual void reactMameFumiProc(dActor_c* player);
    virtual void reactSpinFumiProc(dActor_c* player);
    virtual void reactYoshiFumiProc(dActor_c* yoshi);

    virtual void createModel();
    virtual void createBodyModel();
    virtual void createOtherModel();
    virtual void drawModel();

    virtual void initialize();
    virtual void setTurnByEnemyHit();
    virtual void setTurnByPlayerHit(dActor_c* player);

    virtual void setWalkSpeed();
    virtual void setWalkAnm();
    virtual void playWalkAnm();
    virtual void walkEffect();

    virtual bool isWakidashi() const;
    virtual bool isDamageInvalid();
    virtual bool isBgmSync() const;

    // states
    STATE_VIRTUAL_FUNC_DECLARE(daEnKuriboBase_c, Walk);
    STATE_VIRTUAL_FUNC_DECLARE(daEnKuriboBase_c, Turn);
    STATE_VIRTUAL_FUNC_DECLARE(daEnKuriboBase_c, TrplnJump);

    dHeapAllocator_c mAllocator;

    nw4r::g3d::ResFile mResFile;
    m3d::mdl_c mModel;
    m3d::anmChr_c mAnmChr;
    nw4r::g3d::ResAnmTexPat mResPat;
    m3d::anmTexPat_c mAnmTexPat;

    float mZOffset;
    u32 mIsFrozen;
    float mSavedSpeed;

    mEf::levelEffect_c mLevelEffect;
    mEf::levelOneEffect_c mLevelOneEffect;

    u16 mCheckZoneBoundParam;
};
