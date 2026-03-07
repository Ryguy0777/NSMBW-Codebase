#pragma once

#include <game/bases/d_base.hpp>

/// @unofficial
class dCharacterChangeSelectContents_c : public dBase_c {
public:
    u8 mPad[0x231];
    bool mIsEasyPairingActive;
};