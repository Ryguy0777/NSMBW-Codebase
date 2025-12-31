#pragma once
#include <new/bases/d_a_lift_bg_base.hpp>
#include <game/bases/d_rail.hpp>

class daLiftBgRail_c : public daLiftBgBase_c {
public:
    virtual bool initMovement();
    virtual void executeMovement();

    dRail_c mRail;
    mVec3_c mRailPos;

    // Settings
    u8 mRailIdx;
    u8 mRailIndex;
};