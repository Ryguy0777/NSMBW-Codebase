#pragma once

#include <lib/egg/core/eggDisposer.h>
#include <lib/egg/math/eggVector.h>
#include <lib/egg/core/eggBitFlag.h>
#include <lib/egg/math/eggMatrix.h>
#include <lib/egg/core/eggBuffer.h>
#include <lib/revolution/kpad.h>
#include <lib/revolution/wpad.h>
#include <lib/revolution/pad.h>

namespace EGG {

const int CORE_COUNT = 4;

class CoreStatus : public KPADStatus {
public:
    u32 getFSStickButton() const;

    void init();

    f32 getFSStickX() const {
        return ex_status.fs.stick.x;
    }

    f32 getFSStickY() const {
        return ex_status.fs.stick.y;
    }

    bool down(u32 mask) const {
        return (mask & hold);
    }

    bool up(u32 mask) const {
        return (mask & hold) != mask;
    }

    bool downTrigger(u32 mask) const {
        return (mask & trig);
    }

    bool upTrigger(u32 mask) const {
        return (mask & release);
    }

    bool downAll(u32 mask) const {
        return (mask & hold) == mask;
    }

    bool upAll(u32 mask) const {
        return (mask & hold) == 0;
    }

    u8 getDevType() const {
        return dev_type;
    }

    bool isCore() const {
        return getDevType() == WPAD_DEV_CORE;
    }

    bool isFreestyle() const {
        return getDevType() == WPAD_DEV_FREESTYLE;
    }

    u32 getHold() const {
        return hold;
    }

    s8 getDpdValidFlag() const {
        return dpd_valid_fg;
    };

    Vector2f getAccelVertical() {
        return Vector2f(acc_vertical.x, acc_vertical.y);
    }

    const Vec& getAccel() const {
        return acc;
    }
};

class ControllerRumbleMgr;

class CoreController {
public:
    CoreController();

    virtual void setPosParam(f32 v1, f32 v2);
    virtual void setHoriParam(f32 v1, f32 v2);
    virtual void setDistParam(f32 v1, f32 v2);
    virtual void setAccParam(f32 v1, f32 v2);
    virtual bool down(u32 mask) const;
    virtual bool up(u32 mask) const;
    virtual bool downTrigger(u32 mask) const;
    virtual bool upTrigger(u32 mask) const;
    virtual bool downAll(u32 mask) const;
    virtual bool upAll(u32 mask) const;
    virtual void beginFrame(PADStatus* padStatus);
    virtual void endFrame();

    void sceneReset();
    Vector2f getDpdRawPos();
    void startMotor();
    void stopMotor();
    void createRumbleMgr(u8 numUnits);
    void startPatternRumble(const char* pattern, int duration, bool bGrabActive);
    void startPowerFrameRumble(float param1, int duration, bool bGrabActive);
    void stopRumbleMgr();
    void calc_posture_matrix(Matrix34f& posture, bool checkStable);

    WPADChannel mChannel;
    u32 mHeld;
    u32 mTriggered;
    u32 mReleased;
    u32 m0x14;
    CoreStatus maStatus[16];
    int mReadStatusIdx;
    TBitFlag<u8> mFlag;
    Vector3f mAccel;
    Vector2f mDpdPos;
    u32 mIdleTime;
    u32 mMotorPattern;
    bool mEnableMotor;
    s32 mMotorFrameDuration;
    u8 mMotorPatternLength;
    u8 mMotorPatternPos;
    ControllerRumbleMgr *mRumbleMgr;
    Matrix34f mPostureMatrix;
    Matrix34f mPostureMatrixPrev;
    TBitFlag<u8> mAccelFlags;
    s32 mMaxAccelFrameTime;
    s32 mAccelFrameTime[3];
    f32 mMaxAccelDiff;
    Vector3f mPrevAccel;
};

class CoreControllerMgr {
public:
    struct T__Disposer : Disposer {
        virtual ~T__Disposer();
        static T__Disposer *sStaticDisposer;
    };

    // This gets defined before the vtable
    T__Disposer mDisposer;

    CoreControllerMgr();
    
    virtual void beginFrame();
    virtual void endFrame();

    CoreController* getNthController(int n);

    TBuffer<CoreController*> mControllers;
    u8 mPad[0xE24-0x20];
    TBuffer<u32> mDevTypes;
    u8 mPad2[0xE60-0xE30];

    static CoreControllerMgr *sInstance;
};

class ControllerRumbleMgr {
public:
    void calc();
};

} // namespace EGG