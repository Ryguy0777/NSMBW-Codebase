#pragma once
#include <types.h>
#include <game/mLib/m_vec.hpp>
#include <game/bases/d_ride_ctr.hpp>

class dRideCircle_c : public dRide_ctr_c {
public:
    mVec2_c mCentreOffset;
    mVec2_c mLeftOffset;
    mVec2_c mTopMaybe;
    float mRadius;

    s32 mRightAngleOffset;
    s32 mLeftAngleOffset;
    s16 mPrevRotation;

    bool _72; // ??
    bool _73; // ??
};