#pragma once

#include <game/bases/d_enemy.hpp>

/// @unofficial
class daEnCoinAngle_c : public dEn_c {
public:
    void rotateCoin(); // 0x809E5660

    u8 mUnk[0x5C0];
    fBaseID_e mControllerUniqueID;
};