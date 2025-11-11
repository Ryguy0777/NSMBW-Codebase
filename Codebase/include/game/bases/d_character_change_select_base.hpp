#pragma once

#include <game/bases/d_base.hpp>

/// @unofficial
class dCharacterChangeSelectBase_c : public dBase_c {
public:
    u8 mPad[0x22C];
    bool mIsEasyPairingActive;
};