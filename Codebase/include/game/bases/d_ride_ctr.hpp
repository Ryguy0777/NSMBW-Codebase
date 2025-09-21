#pragma once
#include <types.h>
#include <game/mLib/m_vec.hpp>

class dActor_c;
class dRc_c;

class dRide_ctr_c {
public:
    void* vtable;
    dActor_c* mpOwner;

    dRide_ctr_c* mpNext;
    dRide_ctr_c* mpPrev;

    dRc_c* mpLinkedRc;

    mVec2_c mRight;
    mVec2_c mLeft;
    mVec2_c mLength;
    mVec2_c mLastLeft;

    float mLineLength;
    float mLeftYDelta;

    u32 mFlags;
    s16 mRotation;
    u8 mType; // 0 = dRideRoll_c, 2 = dRide2Point_c, 4 = dRideCircle_c

    bool m_43;
    bool mLinked;

    u8 mChainlinkMode;
    bool m_46;
    u8 mSubType;
    u8 m_48; // unused?
    u8 m_49; // unused?
    u8 mLayer;
    u8 m_4b; // unused?

    static dRide_ctr_c* mEntryN;
    static dRide_ctr_c* mEntryB;
};