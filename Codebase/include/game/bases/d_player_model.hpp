#pragma once
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_effect.hpp>

class dPlayerMdl_c : public dPyMdlBase_c {
public:
    enum MODEL_e {
        MODEL_NORMAL = 0,
        MODEL_SMALL = 1,
        MODEL_PROPELLER = 2,
        MODEL_PENGUIN = 3,

        MODEL_COUNT = 4,
    };

    struct ModelData {
        const char* mResName;
        const char* mAnmResName;
        const char* mBodyModels[MODEL_COUNT];
        const char* mHeadModels[MODEL_COUNT];
        float mUnkFloat1;
        float mUnkFloat2;
        float mUnkFloat3;
        float mUnkFloat4;
    };

    class mdl_inf_c {
    public:
        m3d::mdl_c mBody;
        m3d::mdl_c mHead;
    };

    dPlayerMdl_c(u8 index);
    virtual ~dPlayerMdl_c();

    virtual int getFaceJointIdx() const;
    virtual void createModel();
    virtual void initialize();
    virtual void play();
    virtual void _calc();
    virtual void calc2();
    virtual void draw();
    virtual m3d::mdl_c *getBodyMdl();
    virtual const void *getAnmResFile() const;
    virtual void setPlayerMode(int);
    virtual void setColorType(u8 colorType);
    virtual void setDark(int);
    virtual void onStarAnm();
    virtual void offStarAnm();
    virtual bool getHeadPropelJointMtx(mMtx_c*);
    virtual void vf58(); ///< @unofficial
    virtual s32 vf60(); ///< @unofficial
    virtual void copyAnm();
    virtual void vf70(); ///< @unofficial
    virtual void vf74(); ///< @unofficial
    virtual void setAnmBind();
    virtual void vf7c(); ///< @unofficial
    virtual void setTexAnmType(TexAnmType_e);
    virtual void setPropelRollSpeed(u16);
    virtual s16 getPropelRollSpeed();
    virtual void setPropelRollAngle(s16);
    virtual s16 getPropelRollAngle();
    virtual void setPropelScale(float);
    virtual float *getLegLengthP(u8);

    virtual void setCurrentModel(MODEL_e model); ///< @unofficial
    virtual void callbackTimingA(nw4r::g3d::ChrAnmResult *, nw4r::g3d::ResNode);
    virtual float vfb4(); ///< @unofficial
    virtual void createPlayerModel();

    m3d::anmTexPat_c* getHeadTexAnm();

    nw4r::g3d::ResFile mModelResFile;
    nw4r::g3d::ResFile mCommonAnimResFile;
    nw4r::g3d::ResFile mAnimResFile;

    mEf::levelEffect_c mEffectClass0;
    mEf::levelEffect_c mEffectClass1;

    mdl_inf_c mModels[MODEL_COUNT];

    m3d::anmTexPat_c mBodySwitchAnim;
    m3d::anmMatClr_c mBodyStarAnim;

    m3d::anmTexPat_c mHeadSwitchAnim;
    m3d::anmTexPat_c mPropelHeadSwitchAnim;
    m3d::anmTexPat_c mPenguinHeadSwitchAnim;
    m3d::anmMatClr_c mHeadStarAnim;

    MODEL_e mModelIdx;
    MODEL_e mLastModelIdx;
    
    const ModelData *mpModelData;
    u32 mFaceJointIdx;
    s16 mPropelRollAngle;
    u16 mPropelRollSpeed;
    float mPropelScale;

    u8 mPad[156];
};

class dMarioMdl_c : public dPlayerMdl_c {
public:
    dMarioMdl_c(u8 index);
    virtual ~dMarioMdl_c();

    void intialize();
    void play();
    void _calc();
    void draw();
    void setColorType(u8 colorType);
    s32 vf60(); ///< @unofficial
    void setTexAnmType(TexAnmType_e);
    void vfac(); ///< @unofficial

    void setCurrentModel(MODEL_e model); ///< @unofficial
    float vfb4(); ///< @unofficial
    void createPlayerModel();

    u32 m_00;
    m3d::smdl_c mCapModels[2];
    m3d::anmTexPat_c mCapPat;
    BOOL mIsHatless;
    u32 mCapIdx;
};

class dKinopioMdl_c : public dPlayerMdl_c {
public:
    dKinopioMdl_c(u8 index);
    virtual ~dKinopioMdl_c();

    void intialize();
    void play();
    void _calc();
    void draw();
    void setColorType(u8 colorType);
    s32 vf60(); ///< @unofficial
    void vf7c(); ///< @unofficial
    void setTexAnmType(TexAnmType_e);
    
    void callbackTimingA(nw4r::g3d::ChrAnmResult *, nw4r::g3d::ResNode);
    float vfb4(); ///< @unofficial
    void createPlayerModel();

    m3d::anmTexPat_c mPropelBodySwitchAnim;
    m3d::anmTexPat_c mPenguinBodySwitchAnim;
};
