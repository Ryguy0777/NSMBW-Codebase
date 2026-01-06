#pragma once

#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_allocator.hpp>

class dPyMdlBase_c {
public:
    enum TexAnmType_e { };

    ///< @unofficial
    enum ChrAnmType_e {
        WAIT = 0,
        WALK = 1,
        RUN = 2,
        B_DASH = 3,
        B_DASH2 = 4,
        JUMP = 5,
        JUMP2 = 6,
        JUMPED = 7,
        _2JMP_C_1 = 8,
        _2JMP_C_2 = 9,
        _2JUMPED = 10,
        ROLL_JUMP = 11,
        _2JUMP2 = 12,
        MAME_JUMP2 = 13,
        TURN = 14,
        TURNED = 15,
        HIPSR = 16,
        HIPAT = 17,
        HIPED = 18,
        HIP_TO_STOOP = 19,
        STOOP = 20,
        STOOP_START = 21,
        SLIP = 22,
        MONKEY_START = 0x2F,
        MONKEY_WAIT_R = 0x30,
        MONKEY_WAIT_L = 0x31,
        MONKEY_R_TO_L = 0x32,
        MONKEY_L_TO_R = 0x33,
        GOAL_PUTON_CAP = 0x5A,
        GOAL_PUTON_CAP2 = 0x5C,
        STAR_ROLL = 0x75,
        SJUMPED = 0x78,
        DEMO_TALK = 0xAB,
        BUSY_WAIT = 0xAC,
        ENDING_WAIT = 0xAF,
    };

    virtual ~dPyMdlBase_c();
    virtual void getFaceJointIdx();
    virtual void createModel();
    virtual void initialize();
    virtual void play();
    virtual void _calc();
    virtual void calc2();
    virtual void draw();
    virtual m3d::mdl_c *getBodyMdl();
    virtual void getAnmResFile();
    virtual void setPlayerMode(int);
    virtual void setColorType();
    virtual void setDark();
    virtual void vf3c(); ///< @unofficial
    virtual void onStarAnm();
    virtual void offStarAnm();
    virtual void onStarEffect();
    virtual void offStarEffect();
    virtual void getJointMtx();
    virtual void getHeadPropelJointMtx();
    virtual void vf58(); ///< @unofficial
    virtual void setAnm(int, float, float, float);
    virtual void vf60(); ///< @unofficial
    virtual void vf64(); ///< @unofficial
    virtual void vf68(); ///< @unofficial
    virtual void copyAnm();
    virtual void vf70(); ///< @unofficial
    virtual void vf74(); ///< @unofficial
    virtual void setAnmBind();
    virtual void vf7c(); ///< @unofficial
    virtual void setTexAnmType();
    virtual void setFrame(float);
    virtual void setBodyFrame();
    virtual void setRate(float);
    virtual void setBodyRate();
    virtual void vf94(); ///< @unofficial
    virtual s16 getPropelRollSpeed();
    virtual void vf9c(); ///< @unofficial
    virtual void vfa0(); ///< @unofficial
    virtual void vfa4(); ///< @unofficial
    virtual void vfa8(); ///< @unofficial
    virtual void vfac(); ///< @unofficial

    bool isFootStepTiming();
    void getJointPos(mVec3_c *, int);

    float getFrameMax() { return mAnm.mFrameMax; }
    mMtx_c &getMtx() { return mMtx; }

    void setAng(mAng3_c ang) {
        m_1fc = ang.x;
        m_1fe = ang.y;
        m_200 = ang.z;
    }

    mHeapAllocator_c mAllocator;
    u8 mPad1[0x8];
    m3d::anmChr_c mAnm;
    u8 mPad2[0x6c];
    mVec3_c mHatPosMaybe;
    u8 mPad3[0x30];
    mMtx_c mMtx;
    u8 mPad4[0x18];
    u8 mPlayerType;
    u8 mPlayerID;
    u8 mPlayerMode;
    int m_154;
    u8 mPad5[8];
    u32 mFlags;
    u8 mPad6[0x1C];
    u32 mSceneType;
    u8 mPad7[0x78];
    s16 m_1fc;
    s16 m_1fe;
    s16 m_200;
    u32 m_204;
    u32 m_208;

    static const float scWaterCrouchAnmSpeed;
};