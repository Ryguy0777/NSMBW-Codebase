#pragma once
#include <types.h>
#include <game/mLib/m_vec.hpp>
#include <game/bases/d_bc.hpp>

class dBg_ctr_c {
public:
    typedef bool (*checkF)(dActor_c* self, dActor_c* other);
    typedef bool (*checkH)(dActor_c* self, dActor_c* other);
    typedef bool (*checkW)(dActor_c* self, dActor_c* other, u8 direction);

    typedef void (*callbackF)(dActor_c* self, dActor_c* other);
    typedef void (*callbackH)(dActor_c* self, dActor_c* other);
    typedef void (*callbackW)(dActor_c* self, dActor_c* other, u8 direction);

    dActor_c* mpOwner;

    dBg_ctr_c* mpNext;
    dBg_ctr_c* mpPrev;

    dBc_c* mpTriggeredBelowSensor;
    dBc_c* mpTriggeredAboveSensor;
    dBc_c* mpTriggeredAdjSensor;
    dBc_c* mpTriggeredPlayerFenceSensors[4]; // used by players on moving chainlinks and vines
    dBc_c* mpTriggeredPlayerWallSlideSensors[4]; // used by players when wall sliding

    dActor_c* mpCarryingActor; // collisions with this actor are ignored

    u32 mIsCalced;

    callbackF mBelowCallback;
    callbackH mAboveCallback;
    callbackW mAdjCallback;

    checkF mBelowCheckFunc;
    checkH mAoveCheckFunc;
    checkW mAdjCheckFunc;

    mVec2_c mLastPos;
    mVec2_c mCalcedPos[4];

    float mOffsetLeft; // used for circles as well
    float mOffsetTop; // used for circles as well
    float mOffsetRight;
    float mOffsetBottom;

    float mRadius; // only used for circles

    mVec3_c mOwnerPos;
    mVec2_c mDiagonalSize; // ??
    mVec2_c mPrevDiagonalSize; // ??
    mVec3_c mDisplacement; // ??

    s16* mRotationPtr;
    s16 mRotation;
    s16 mRotationDelta;
    s16 mRotationDelta2; // no idea why this is a thing
    // 2 bytes padding

    u32 mIsRound;
    u32 _CC; // value 0-10, maybe a category sort of thing??
    u32 mFlags; // TODO document
    s32 _D4;
    u32 _D8; // a bitfield

    bool mLinked;

    u8 mChainlinkMode;
    u8 mLayer;
    // 1 byte padding

    s32 mId;

    static dBg_ctr_c* mEntryN;
    static dBg_ctr_c* mEntryB;
};