#pragma once
#include <game/bases/d_base_actor.hpp>

/// @unofficial
class da2DPlayer_c : public dBaseActor_c {
public:
    u32 mPad[0x4F];
    bool mIsEasyPairingActive;
};