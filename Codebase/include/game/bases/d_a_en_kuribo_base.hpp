#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_effect.hpp>

class daEnKuriboBase_c : public dEn_c {
public:
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

    // new virtuals
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
    virtual void reactFumiProc(dActor_c* killingActor);
    virtual void vf2ac(); ///< @unofficial
    virtual void reactSpinFumiProc(dActor_c* killingActor); ///< @unofficial
    virtual void reactYoshiFumiProc(dActor_c* killingActor); ///< @unofficial

    virtual void createModel();
    virtual void loadModel(); ///< @unofficial
    virtual void loadWingModel(); ///< @unofficial
    virtual void drawModel();

    virtual void initialize();
    virtual void setTurnState(); ///< @unofficial
    virtual void vf2d0(dActor_c* collidingActor); ///< @unofficial

    virtual void setWalkSpeed();
    virtual void playWalkAnim(); ///< @unofficial
    virtual void updateModel(); ///< @unofficial
    virtual void walkEffect();

    virtual bool isWakidashi() const;
    virtual bool vf2e8(); ///< @unofficial
    virtual bool isBgmSync() const;

    // states
    STATE_VIRTUAL_FUNC_DECLARE(daEnKuriboBase_c, Walk);
    STATE_VIRTUAL_FUNC_DECLARE(daEnKuriboBase_c, Turn);
    STATE_VIRTUAL_FUNC_DECLARE(daEnKuriboBase_c, TrplnJump);
};
