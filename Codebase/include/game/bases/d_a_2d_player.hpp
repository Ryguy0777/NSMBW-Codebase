#pragma once

#include <game/bases/d_base_actor.hpp>

/// @unofficial
class da2DPlayer_c : public dBaseActor_c {
public:
    u8 mPad[0x13C];
    bool mIsEasyPairingActive;
};